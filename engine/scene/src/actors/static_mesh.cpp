#include "scene/actors/static_mesh.hpp"

#include "scene/material.hpp"
#include "scene/world.hpp"

#include "engine/engine.hpp"
#include "schemas/json.hpp"

#include "imgui.h"

namespace lucid::scene
{
    CStaticMesh::CStaticMesh(const FDString& InName,
                             const IActor* InParent,
                             CWorld* InWorld,
                             resources::CMeshResource* InMeshResource,
                             CMaterial* InMaterial,
                             const EStaticMeshType& InType)
    : IActor(InName, InParent, InWorld), MeshResource(InMeshResource), Material(InMaterial), Type(InType)
    {
    }

#if DEVELOPMENT
    void CStaticMesh::UIDrawActorDetails()
    {
        IActor::UIDrawActorDetails();

        ImGui::Text("Static mesh:");
        if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (int i = 0; i < GEngine.GetMeshesHolder().Length(); ++i)
            {
                resources::CMeshResource* CurrMeshResource = GEngine.GetMeshesHolder().GetByIndex(i);
                if(ImGui::Selectable(*CurrMeshResource->GetName(), MeshResource == CurrMeshResource))
                {
                    MeshResource = CurrMeshResource;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (MeshResource == CurrMeshResource)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    void CStaticMesh::FillDescription(FStaticMeshDescription& OutDescription) const
    {
        OutDescription.Type = Type;
        if (BaseStaticMesh)
        {
            if (MeshResource != BaseStaticMesh->MeshResource)
            {
                OutDescription.MeshResourceName.bChanged = true;
                OutDescription.MeshResourceName.Value = *MeshResource->GetName();
            }

            if (Material != BaseStaticMesh->Material)
            {
                OutDescription.MaterialName.bChanged = true;
                OutDescription.MaterialName.Value = *Material->GetName();
            }
        }
        else
        {
            OutDescription.MeshResourceName.Value = *MeshResource->GetName();
            OutDescription.MaterialName.Value = *Material->GetName();
        }

        OutDescription.Id = Id;
        OutDescription.ParentId = Parent ? Parent->Id : 0;
        OutDescription.BaseActorResourceId = BaseStaticMesh ? BaseStaticMesh->ResourceId : sole::INVALID_UUID;
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
        if (0.002f > fabs(MeshResource->MinPosY))
        {
            return (MeshResource->MaxPosY - MeshResource->MinPosY) * Transform.Scale.y / 2.f;
        }
        return 0;
    }

    CStaticMesh* CStaticMesh::CreateActor(CStaticMesh const* BaseActorResource, CWorld* InWorld, const FStaticMeshDescription& InStaticMeshDescription)
    {
        auto* Parent = InStaticMeshDescription.ParentId > 0 ? InWorld->GetActorById(InStaticMeshDescription.ParentId) : nullptr;

        resources::CMeshResource*   MeshResource;
        if (InStaticMeshDescription.MeshResourceName.bChanged)
        {
            MeshResource = GEngine.GetMeshesHolder().Get(*InStaticMeshDescription.MeshResourceName.Value);
        }
        else
        {
            MeshResource = BaseActorResource->MeshResource;
        }

        CMaterial*            Material;
        if (InStaticMeshDescription.MaterialName.bChanged)
        {
            Material = GEngine.GetMaterialsHolder().Get(*InStaticMeshDescription.MaterialName.Value);
        }
        else
        {
            Material = BaseActorResource->Material;
        }

        auto* StaticMesh = new CStaticMesh { InStaticMeshDescription.Name, Parent, InWorld, MeshResource, Material, InStaticMeshDescription.Type };
        StaticMesh->Transform.Translation = Float3ToVec(InStaticMeshDescription.Postion);
        StaticMesh->Transform.Rotation = Float4ToQuat(InStaticMeshDescription.Rotation);
        StaticMesh->Transform.Scale = Float3ToVec(InStaticMeshDescription.Scale);
        StaticMesh->SetReverseNormals(InStaticMeshDescription.bReverseNormals);
        StaticMesh->BaseStaticMesh = BaseActorResource;
        
        if (InWorld)
        {
            InWorld->AddStaticMesh(StaticMesh);            
        }

        return StaticMesh;
    }
} // namespace lucid::scene
