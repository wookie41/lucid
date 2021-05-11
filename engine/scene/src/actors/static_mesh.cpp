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
        if (BaseActorAsset)
        {
            // React to BaseActorAsset being changed in IActor::UIDrawActorDetails
            if (PrevBaseActorAsset)
            {
                if (auto* OldStaticMesh = dynamic_cast<CStaticMesh*>(PrevBaseActorAsset))
                {
                    OldStaticMesh->ChildReferences.Remove(this);
                }

                if (auto* NewStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorAsset))
                {
                    MeshResource = NewStaticMesh->MeshResource;
                    NewStaticMesh->ChildReferences.Add(this);
                    BaseStaticMesh = NewStaticMesh;
                    UpdateMaterialSlots(NewStaticMesh);
                }
            }
        }

        if (ImGui::CollapsingHeader("Static mesh"))
        {
            // Handle actor instance details
            ImGui::Checkbox("Reverse normals:", &bReverseNormals);

            if (ResourceId == sole::INVALID_UUID)
            {
                // Actor instance editing
                ImGui::Text("Override mesh resource:");
                ImGui::SameLine();
                if (ImGui::Button("Revert to base"))
                {
                    if (auto* BaseStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorAsset))
                    {
                        MeshResource = BaseStaticMesh->MeshResource;
                        UpdateMaterialSlots(BaseStaticMesh);
                    }
                }
                ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);
            }
            else
            {
                // Actor asset editing
                resources::CMeshResource* OldMesh = MeshResource;
                ImGui::Text("Mesh resource:");
                ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);

                // Refresh mesh resource on child references if they don't override the mesh
                if (OldMesh != MeshResource)
                {
                    HandleBaseAssetMeshResourceChange(OldMesh);
                }
            }

            resources::CMeshResource* OldMesh = MeshResource;
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
                if (!BaseActorAsset)
                {
                    HandleBaseAssetNumMaterialsChange(true);
                }
            }
            ImGui::SameLine(0, 4);
            if (ImGui::Button("-"))
            {
                MaterialSlots.RemoveLast();
                if (!BaseActorAsset)
                {
                    HandleBaseAssetNumMaterialsChange(true);
                }
            }

            // Material editors for material slots
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                auto MaterialSlotEditorLabel = SPrintf("static_mesh_material_%d", i);
                auto* Material = *MaterialSlots[i];
                if (ImGui::TreeNode(*MaterialSlotEditorLabel, "Material slot %d: %s", i,
                                    Material ? *Material->GetName() : "-- None --"))
                {
                    if (ImGui::Button("Edit"))
                    {
                        CurrentlyEditedMaterial = Material;
                    }

                    if (CurrentlyEditedMaterial)
                    {
                        ImGuiShowMaterialEditor(Material, &bMaterialEditorOpen);
                        if (!bMaterialEditorOpen)
                        {
                            CurrentlyEditedMaterial = nullptr;
                        }
                    }

                    ImGui::Text("Select different:");
                    CMaterial* OldMaterial = *MaterialSlots[i];
                    ImGuiMaterialPicker(*MaterialSlotEditorLabel, MaterialSlots[i]);
                    if (!OldMaterial || OldMaterial->GetID() != (*MaterialSlots[i])->GetID())
                    {
                        if (BaseActorAsset)
                        {
                            if (OldMaterial)
                            {
                                delete OldMaterial;
                            }
                            *MaterialSlots[i] = (*MaterialSlots[i])->GetCopy();
                        }
                        else
                        {
                            HandleBaseAssetMaterialSlotChange(OldMaterial, i);
                        }
                    }
                    else
                    {
                        *MaterialSlots[i] = OldMaterial;
                    }

                    ImGui::TreePop();
                }

                MaterialSlotEditorLabel.Free();
            }
        }
    }

    void CStaticMesh::HandleBaseAssetMeshResourceChange(resources::CMeshResource* OldMesh)
    {
        auto ChildReference = &ChildReferences.Head;
        while (ChildReference && ChildReference->Element)
        {
            auto* ChildStaticMeshRef = ChildReference->Element;
            // Replace mesh only if the child didn't override it
            if (ChildStaticMeshRef->MeshResource == OldMesh)
            {
                ChildStaticMeshRef->MeshResource = MeshResource;
            }
            ChildReference = ChildReference->Next;
        }
    }

    void CStaticMesh::HandleBaseAssetNumMaterialsChange(const bool& bAdded)
    {
        auto ChildReference = &ChildReferences.Head;
        while (ChildReference && ChildReference->Element)
        {
            auto* ChildStaticMeshRef = ChildReference->Element;
            // Modify only if the child didn't override the mesh
            if (ChildStaticMeshRef->MeshResource == MeshResource)
            {
                if (bAdded)
                {
                    ChildStaticMeshRef->MaterialSlots.Add(nullptr);
                }
                else
                {
                    ChildStaticMeshRef->MaterialSlots.RemoveLast();
                }
            }
            ChildReference = ChildReference->Next;
        }
    }

    void CStaticMesh::HandleBaseAssetMaterialSlotChange(CMaterial* InOldMaterial, const u8& InMaterialSlot)
    {
        auto ChildReference = &ChildReferences.Head;
        while (ChildReference && ChildReference->Element)
        {
            auto* ChildStaticMeshRef = ChildReference->Element;
            // Replace only if the child didn't override the mesh and material at that slot
            if (ChildStaticMeshRef->MeshResource == MeshResource)
            {
                auto* ChildMaterialSlot = ChildStaticMeshRef->GetMaterialSlot(InMaterialSlot);
                if (ChildMaterialSlot->GetID() == InOldMaterial->GetID())
                {
                    delete *ChildStaticMeshRef->MaterialSlots[InMaterialSlot];
                    *ChildStaticMeshRef->MaterialSlots[InMaterialSlot] = (*MaterialSlots[InMaterialSlot])->GetCopy();
                }
            }
            ChildReference = ChildReference->Next;
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
                if (i < MaterialSlots.GetLength() && *MaterialSlots[i] && *MaterialSlots[i] != *(BaseStaticMesh->
                    MaterialSlots[i]))
                {
                    OutDescription.MaterialIds.push_back({(*MaterialSlots[i])->GetID(), true});
                }
                else
                {
                    OutDescription.MaterialIds.push_back({sole::INVALID_UUID, false});
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
                    {*MaterialSlots[i] ? (*MaterialSlots[i])->GetID() : sole::INVALID_UUID, true});
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

    void CStaticMesh::InternalSaveToResourceFile(const FString& InFilePath)
    {
        FStaticMeshDescription StaticMeshDescription;
        FillDescription(StaticMeshDescription);
        WriteToJSONFile(StaticMeshDescription, *InFilePath);
        GEngine.GetActorsDatabase().Entries.push_back({ResourceId, ResourcePath, EActorType::STATIC_MESH});
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

    CStaticMesh* CStaticMesh::CreateActor(CStaticMesh* BaseActorResource,
                                          CWorld* InWorld,
                                          const FStaticMeshDescription& InStaticMeshDescription)
    {
        assert(BaseActorResource);
        assert(InWorld);

        auto* Parent = InStaticMeshDescription.ParentId == 0
                           ? nullptr
                           : InWorld->GetActorById(InStaticMeshDescription.ParentId);

        resources::CMeshResource* MeshResource;
        if (InStaticMeshDescription.MeshResourceId.bChanged)
        {
            MeshResource = GEngine.GetMeshesHolder().Get(InStaticMeshDescription.MeshResourceId.Value);
        }
        else
        {
            MeshResource = BaseActorResource->MeshResource;
        }

        auto* StaticMesh = new CStaticMesh{
            InStaticMeshDescription.Name, Parent, InWorld, MeshResource, InStaticMeshDescription.Type
        };
        StaticMesh->Id = InStaticMeshDescription.Id;
        StaticMesh->Transform.Translation = Float3ToVec(InStaticMeshDescription.Postion);
        StaticMesh->Transform.Rotation = Float4ToQuat(InStaticMeshDescription.Rotation);
        StaticMesh->Transform.Scale = Float3ToVec(InStaticMeshDescription.Scale);
        StaticMesh->bVisible = InStaticMeshDescription.bVisible;
        StaticMesh->SetReverseNormals(InStaticMeshDescription.bReverseNormals);
        StaticMesh->BaseActorAsset = BaseActorResource;
        StaticMesh->BaseStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorResource);

        for (u16 i = 0; i < BaseActorResource->GetNumMaterialSlots(); ++i)
        {
            if (i < InStaticMeshDescription.MaterialIds.size() && InStaticMeshDescription.MaterialIds[i].bChanged)
            {
                StaticMesh->AddMaterial(
                    GEngine.GetMaterialsHolder().Get(InStaticMeshDescription.MaterialIds[i].Value)->GetCopy());
            }
            else
            {
                StaticMesh->AddMaterial(BaseActorResource->GetMaterialSlot(i)->GetCopy());
            }
        }

        InWorld->AddStaticMesh(StaticMesh);
        BaseActorResource->ChildReferences.Add(StaticMesh);

        return StaticMesh;
    }

    void CStaticMesh::UpdateMaterialSlots(CStaticMesh const* BaseStaticMesh)
    {
        if (BaseStaticMesh)
        {
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                if (*MaterialSlots[i])
                {
                    delete *MaterialSlots[i];
                }
            }

            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{BaseStaticMesh->GetNumMaterialSlots(), true};

            for (int i = 0; i < BaseStaticMesh->GetNumMaterialSlots(); ++i)
            {
                MaterialSlots.Add(BaseStaticMesh->GetMaterialSlot(i)->GetCopy());
            }
        }
        else
        {
            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{1, true};
        }
    }

    IActor* CStaticMesh::CreateActorAsset(const FDString& InName) const
    {
        auto* ActorAsset = new CStaticMesh{InName, nullptr, nullptr, MeshResource, EStaticMeshType::STATIONARY};
        ActorAsset->MaterialSlots = FArray<CMaterial*>(MaterialSlots.GetLength(), true);
        for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            ActorAsset->MaterialSlots.Add(GEngine.GetMaterialsHolder().Get(GetMaterialSlot(i)->GetID()));
        }
        return ActorAsset;
    }

    CStaticMesh* CStaticMesh::CreateEmptyActorAsset(const FDString& InName)
    {
        auto* EmptyActorAsset = new CStaticMesh{InName, nullptr, nullptr, nullptr, EStaticMeshType::STATIONARY};
        EmptyActorAsset->MaterialSlots = FArray<CMaterial*>(1, true);
        return EmptyActorAsset;
    }

    void CStaticMesh::LoadAsset()
    {
        assert(ResourcePath.GetLength());

        FStaticMeshDescription StaticMeshDescription;
        if (ReadFromJSONFile(StaticMeshDescription, *ResourcePath)) // @TODO strings from here don't get freed
        {
            MeshResource = GEngine.GetMeshesHolder().Get(StaticMeshDescription.MeshResourceId.Value);
            for (u16 i = 0; i < StaticMeshDescription.MaterialIds.size(); ++i)
            {
                MaterialSlots.Add(GEngine.GetMaterialsHolder().Get(StaticMeshDescription.MaterialIds[i].Value));
            }
        }
        else
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to load asset %s - couldn't read file %s", *Name, *ResourcePath)
        }
    }

    IActor* CStaticMesh::CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        auto* SpawnedMesh = new CStaticMesh{Name.GetCopy(), nullptr, InWorld, MeshResource, Type};
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            SpawnedMesh->AddMaterial((*MaterialSlots[i])->GetCopy());
        }
        SpawnedMesh->Transform.Translation = InSpawnPosition;
        SpawnedMesh->BaseActorAsset = this;
        SpawnedMesh->BaseStaticMesh = this;
        ChildReferences.Add(SpawnedMesh);
        InWorld->AddStaticMesh(SpawnedMesh);
        return SpawnedMesh;
    }

    IActor* CStaticMesh::CreateCopy()
    {
        auto* SpawnedMesh = new CStaticMesh{Name.GetCopy(), Parent, World, MeshResource, Type};
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            SpawnedMesh->AddMaterial((*MaterialSlots[i])->GetCopy());
        }
        SpawnedMesh->Transform = Transform;
        SpawnedMesh->Transform.Translation += glm::vec3{1, 0, 0};
        SpawnedMesh->BaseActorAsset = BaseActorAsset;
        SpawnedMesh->BaseStaticMesh = BaseStaticMesh;
        BaseStaticMesh->ChildReferences.Add(SpawnedMesh);
        World->AddStaticMesh(SpawnedMesh);
        return SpawnedMesh;
    }

    void CStaticMesh::OnRemoveFromWorld()
    {
        IActor::OnRemoveFromWorld();
        World->RemoveStaticMesh(Id);
        
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            if (auto* MaterialSlot = *MaterialSlots[i])
            {
                delete MaterialSlot;
            }   
        }
    }
} // namespace lucid::scene
