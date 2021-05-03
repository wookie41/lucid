#include "scene/actors/static_mesh.hpp"

#include "scene/material.hpp"
#include "scene/world.hpp"

#include "engine/engine.hpp"
#include "schemas/json.hpp"

#if DEVELOPMENT
#include "imgui.h"
#include "imgui_lucid.h"
#endif

namespace lucid::scene
{
    CStaticMesh::CStaticMesh(const FDString& InName,
                             IActor* InParent,
                             CWorld* InWorld,
                             resources::CMeshResource* InMeshResource,
                             const EStaticMeshType& InType)
    : IActor(InName, InParent, InWorld), MeshResource(InMeshResource), Type(InType)
    {
    }

#if DEVELOPMENT
    void CStaticMesh::UIDrawActorDetails()
    {
        IActor::UIDrawActorDetails();

        ImGui::Text("Static mesh:");
        ImGui::Checkbox("Reverse normals", &bReverseNormals);
        ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);
        for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            ImGui::Text("Material slot %d:", i);
            ImGuiMaterialPicker("static_mesh_material", MaterialSlots[i]);
            if (*MaterialSlots[i])
            {
                (*MaterialSlots[i])->UIDrawMaterialEditor();                
            }
            else
            {
                ImGui::Text("-- No material selected --");
            }
        }
    }

    void CStaticMesh::FillDescription(FStaticMeshDescription& OutDescription) const
    {
        OutDescription.Type = Type;
        if (BaseStaticMesh)
        {
            if (MeshResource != BaseStaticMesh->MeshResource)
            {
                OutDescription.MeshResourceId.bChanged = true;
                OutDescription.MeshResourceId.Value = MeshResource->GetID();
            }

            for (u16 i = 0; i < BaseStaticMesh->MaterialSlots.GetLength(); ++i)
            {
                if (*MaterialSlots[i] != *(BaseStaticMesh->MaterialSlots[i]))
                {
                    OutDescription.MaterialIds.push_back({(*MaterialSlots[i])->GetID(), true });
                }
                else
                {
                    OutDescription.MaterialIds.push_back({sole::INVALID_UUID, false });
                }
            }
            
            OutDescription.BaseActorResourceId = BaseStaticMesh->ResourceId;
        }
        else
        {
            OutDescription.MeshResourceId.Value = MeshResource->GetID();
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                OutDescription.MaterialIds.push_back({(*MaterialSlots[i])->GetID(), true});
            }
        }

        OutDescription.Id = Id;
        OutDescription.ParentId = Parent ? Parent->Id : 0;
        OutDescription.Name = Name;
        OutDescription.Postion = VecToFloat3(Transform.Translation);
        OutDescription.Rotation = QuatToFloat4(Transform.Rotation);
        OutDescription.Scale = VecToFloat3(Transform.Scale);
        OutDescription.bVisible = bVisible;
        OutDescription.bReverseNormals = bReverseNormals;
    }
    
    void CStaticMesh::_SaveToResourceFile(const FString& InFilePath)
    {
        FStaticMeshDescription StaticMeshDescription;
        FillDescription(StaticMeshDescription);
        WriteToJSONFile(StaticMeshDescription, *InFilePath);        
    }
    
#endif
    float CStaticMesh::GetVerticalMidPoint() const
    {
#if DEVELOPMENT
        if (!MeshResource)
        {
            return 0;
        }
#endif
        if (0.002f > fabs(MeshResource->MinPosY))
        {
            return (MeshResource->MaxPosY - MeshResource->MinPosY) * Transform.Scale.y / 2.f;
        }
        return 0;
    }

    CStaticMesh* CStaticMesh::CreateActor(CStaticMesh const* BaseActorResource, CWorld* InWorld, const FStaticMeshDescription& InStaticMeshDescription)
    {
        auto* Parent = InStaticMeshDescription.ParentId > 0 ? InWorld->GetActorById(InStaticMeshDescription.ParentId) : nullptr;

        resources::CMeshResource* MeshResource;
        if (InStaticMeshDescription.MeshResourceId.bChanged)
        {
            MeshResource = GEngine.GetMeshesHolder().Get(InStaticMeshDescription.MeshResourceId.Value);
        }
        else
        {
            MeshResource = BaseActorResource->MeshResource;
        }

        auto* StaticMesh = new CStaticMesh { InStaticMeshDescription.Name, Parent, InWorld, MeshResource, InStaticMeshDescription.Type };
        StaticMesh->Transform.Translation = Float3ToVec(InStaticMeshDescription.Postion);
        StaticMesh->Transform.Rotation = Float4ToQuat(InStaticMeshDescription.Rotation);
        StaticMesh->Transform.Scale = Float3ToVec(InStaticMeshDescription.Scale);
        StaticMesh->SetReverseNormals(InStaticMeshDescription.bReverseNormals);
        StaticMesh->BaseStaticMesh = BaseActorResource;

        if (BaseActorResource)
        {
            for (u16 i = 0; i < BaseActorResource->GetNumMaterialSlots(); ++i)
            {
                if (i < InStaticMeshDescription.MaterialIds.size() && InStaticMeshDescription.MaterialIds[i].bChanged)
                {
                    StaticMesh->AddMaterial(GEngine.GetMaterialsHolder().Get(InStaticMeshDescription.MaterialIds[i].Value));
                }
                else
                {
                    StaticMesh->AddMaterial(BaseActorResource->GetMaterialSlot(i));

                }            
            }
        }
        else
        {
            for (u16 i = 0; i < InStaticMeshDescription.MaterialIds.size(); ++i)
            {
                StaticMesh->AddMaterial(GEngine.GetMaterialsHolder().Get(InStaticMeshDescription.MaterialIds[i].Value));
            }
        }

        if (InWorld)
        {
            InWorld->AddStaticMesh(StaticMesh);            
        }

        return StaticMesh;
    }
} // namespace lucid::scene
