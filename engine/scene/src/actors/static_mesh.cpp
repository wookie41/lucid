#include "scene/actors/static_mesh.hpp"

#include <resources/mesh_resource.hpp>
#include <sole/sole.hpp>

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

        // Handle actor instance details
        if (ResourceId == sole::INVALID_UUID)
        {
            // React to BaseActorAsset being changed in IActor::UIDrawActorDetails
            if (bBaseActorAssetChanged)
            {
                if (auto* NewStaticMesh = dynamic_cast<CStaticMesh const*>(BaseActorAsset))
                {
                    MeshResource = NewStaticMesh->MeshResource;
                    UpdateMaterialSlots(NewStaticMesh);
                }
            }
        }
        ImGui::Checkbox("Reverse normals:", &bReverseNormals);

        resources::CMeshResource* OldMesh = MeshResource;

        if (ResourceId == sole::INVALID_UUID)
        {
            ImGui::Text("Override mesh resource:");
            ImGui::SameLine();
            if (ImGui::Button("Revert to base"))
            {
                if (auto* BaseStaticMesh = dynamic_cast<CStaticMesh const*>(BaseActorAsset))
                {
                    MeshResource = BaseStaticMesh->MeshResource;
                    UpdateMaterialSlots(BaseStaticMesh);
                }
            }
            ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);
        }
        else
        {
            ImGui::Text("Mesh resource:");
            ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);
        }

        if (MeshResource != OldMesh)
        {
            if (MeshResource == nullptr)
            {
                if (auto* BaseStaticMesh = dynamic_cast<CStaticMesh const*>(BaseActorAsset))
                {
                    MeshResource = BaseStaticMesh->MeshResource;
                }
                else
                {
                    MeshResource = OldMesh;
                }
            }
            UpdateMaterialSlots(nullptr);
        }
        else
        {
            MeshResource = OldMesh;
        }

        static CMaterial* CurrentlyEditedMaterial = nullptr;
        static bool bMaterialEditorOpen = true;

        // Buttons to add and remove material slots
        ImGui::Text("Num material slots: %d", GetNumMaterialSlots());
        ImGui::SameLine(0, 4);
        if (ImGui::Button("+"))
        {
            MaterialSlots.Add(nullptr);
        }
        ImGui::SameLine(0, 4);
        if (ImGui::Button("-"))
        {
            MaterialSlots.RemoveLast();
        }

        // Material editors for material slots
        for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            auto MaterialSlotEditorLabel = SPrintf("static_mesh_material_%d", i);
            if (ImGui::TreeNode(*MaterialSlotEditorLabel, "Material slot %d:", i))
            {
                if (ImGui::Button("Edit"))
                {
                    CurrentlyEditedMaterial = *MaterialSlots[i];
                }

                if (CurrentlyEditedMaterial)
                {
                    ImGuiShowMaterialEditor(*MaterialSlots[i], &bMaterialEditorOpen);
                    if (!bMaterialEditorOpen)
                    {
                        CurrentlyEditedMaterial = nullptr;
                    }
                }

                ImGui::Text("Select different:");
                ImGuiMaterialPicker(*MaterialSlotEditorLabel, MaterialSlots[i]);
                ImGui::TreePop();
            }

            MaterialSlotEditorLabel.Free();
        }
    }

    void CStaticMesh::FillDescription(FStaticMeshDescription& OutDescription) const
    {
        OutDescription.Type = Type;
        if (auto* BaseStaticMesh = dynamic_cast<CStaticMesh const*>(BaseActorAsset))
        {
            if (MeshResource != BaseStaticMesh->MeshResource)
            {
                OutDescription.MeshResourceId.bChanged = true;
                OutDescription.MeshResourceId.Value = MeshResource->GetID();
            }

            for (u16 i = 0; i < BaseStaticMesh->MaterialSlots.GetLength(); ++i)
            {
                if (i < MaterialSlots.GetLength() && *MaterialSlots[i] &&
                    *MaterialSlots[i] != *(BaseStaticMesh->MaterialSlots[i]))
                {
                    OutDescription.MaterialIds.push_back({ (*MaterialSlots[i])->GetID(), true });
                }
                else
                {
                    OutDescription.MaterialIds.push_back({ sole::INVALID_UUID, false });
                }
            }

            OutDescription.BaseActorResourceId = BaseStaticMesh->ResourceId;
        }
        else
        {
            OutDescription.MeshResourceId.Value = MeshResource ? MeshResource->GetID() : sole::INVALID_UUID;
            OutDescription.MeshResourceId.bChanged = true;
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                OutDescription.MaterialIds.push_back(
                  { *MaterialSlots[i] ? (*MaterialSlots[i])->GetID() : sole::INVALID_UUID, true });
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
        GEngine.GetActorsDatabase().Entries.push_back({ ResourceId, ResourcePath, EActorType::STATIC_MESH });
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

    CStaticMesh* CStaticMesh::CreateActor(CStaticMesh const* BaseActorResource,
                                          CWorld* InWorld,
                                          const FStaticMeshDescription& InStaticMeshDescription)
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

        auto* StaticMesh = new CStaticMesh{ InStaticMeshDescription.Name, Parent, InWorld, MeshResource, InStaticMeshDescription.Type };
        StaticMesh->Transform.Translation = Float3ToVec(InStaticMeshDescription.Postion);
        StaticMesh->Transform.Rotation = Float4ToQuat(InStaticMeshDescription.Rotation);
        StaticMesh->Transform.Scale = Float3ToVec(InStaticMeshDescription.Scale);
        StaticMesh->bVisible = InStaticMeshDescription.bVisible;
        StaticMesh->SetReverseNormals(InStaticMeshDescription.bReverseNormals);
        StaticMesh->BaseActorAsset = BaseActorResource;

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

    void CStaticMesh::UpdateMaterialSlots(CStaticMesh const* BaseStaticMesh)
    {
        if (BaseStaticMesh)
        {
            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{ BaseStaticMesh->GetNumMaterialSlots(), true };

            for (int i = 0; i < BaseStaticMesh->GetNumMaterialSlots(); ++i)
            {
                MaterialSlots.Add(BaseStaticMesh->GetMaterialSlot(i));
            }
        }
        else
        {
            MaterialSlots = FArray<CMaterial*>{ 1, true };
        }
    }


    IActor* CStaticMesh::CreateActorAsset(const FDString& InName) const
    {
        auto* ActorAsset =  new CStaticMesh { InName, nullptr, nullptr, MeshResource, EStaticMeshType::STATIONARY };
        ActorAsset->MaterialSlots = FArray<CMaterial*>(MaterialSlots.GetLength(), true);
        for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            ActorAsset->MaterialSlots.Add(GetMaterialSlot(i));
        }
        return ActorAsset;
    }

} // namespace lucid::scene
