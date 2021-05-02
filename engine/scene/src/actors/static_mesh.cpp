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
        ImGui::Checkbox("Reverse normals", &bReverseNormals);
        ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);
        ImGui::Text("Material:");
        ImGuiMaterialPicker("static_mesh_material", &Material);
        Material->UIDrawMaterialEditor();
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

            if (Material != BaseStaticMesh->Material)
            {
                OutDescription.MaterialId.bChanged  = true;
                OutDescription.MaterialId.Value     = Material->GetID();
            }

            OutDescription.BaseActorResourceId = BaseStaticMesh->ResourceId;
        }
        else
        {
            OutDescription.MeshResourceId.Value = MeshResource->GetID();
            OutDescription.MaterialId.Value     =     Material->GetID();
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

        CMaterial* Material;
        if (InStaticMeshDescription.MaterialId.bChanged)
        {
            Material = GEngine.GetMaterialsHolder().Get(InStaticMeshDescription.MaterialId.Value);
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
