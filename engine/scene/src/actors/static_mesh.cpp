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
    CStaticMesh::CStaticMesh(const FDString&           InName,
                             IActor*                   InParent,
                             CWorld*                   InWorld,
                             resources::CMeshResource* InMeshResource,
                             const EStaticMeshType&    InType)
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
                    OldStaticMesh->RemoveChildReference(this);
                }

                if (auto* NewStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorAsset))
                {
                    MeshResource = NewStaticMesh->MeshResource;
                    NewStaticMesh->AddChildReference(this);
                    BaseStaticMesh = NewStaticMesh;
                    UpdateMaterialSlots(NewStaticMesh);
                }
            }
        }

        if (ImGui::CollapsingHeader("Static mesh"))
        {
            // Handle actor instance details
            ImGui::Checkbox("Reverse normals:", &bReverseNormals);

            if (BaseStaticMesh)
            {
                // Actor instance editing
                ImGui::Text("Override mesh resource:");
                ImGui::SameLine();
                if (ImGui::Button("Revert to base"))
                {
                    if (auto* BaseStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorAsset))
                    {
                        if (MeshResource != BaseStaticMesh->MeshResource)
                        {
                            MeshResource->Release();
                            MeshResource = BaseStaticMesh->MeshResource;
                            UpdateMaterialSlots(BaseStaticMesh);
                        }
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

                if (OldMesh != MeshResource)
                {
                    OldMesh->Release();
                    MeshResource->Acquire(false, true);
                    HandleBaseAssetMeshResourceChange(OldMesh);
                }
            }

            static bool       bMaterialEditorOpen     = true;
            static CMaterial* CurrentlyEditedMaterial = nullptr;

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
                const u8 MaterialSlot = GetNumMaterialSlots() - 1;
                if (CMaterial* LastMaterial = GetMaterialSlot(MaterialSlot))
                {
                    LastMaterial->UnloadResources();

                    if (!BaseActorAsset)
                    {
                        HandleBaseAssetNumMaterialsChange(false);
                    }
                    else if (LastMaterial != BaseStaticMesh->GetMaterialSlot(MaterialSlot))
                    {
                        delete LastMaterial;
                    }
                }
                MaterialSlots.RemoveLast();
            }

            // Material editors for material slots
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                CMaterial* Material                = *MaterialSlots[i];
                FDString   MaterialSlotEditorLabel = SPrintf("static_mesh_material_%d", i);
                if (ImGui::TreeNode(*MaterialSlotEditorLabel, "Material slot %d: %s", i, Material ? *Material->GetName() : "-- None --"))
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
                                // Check if the material wasn't already changed in this mesh
                                if (BaseStaticMesh->GetMaterialSlot(i) != OldMaterial)
                                {
                                    OldMaterial->UnloadResources();
                                }
                                delete OldMaterial;
                            }
                            
                            *MaterialSlots[i] = (*MaterialSlots[i])->GetCopy();
                            (*MaterialSlots[i])->LoadResources();
                        }
                        else
                        {
                            OldMaterial->UnloadResources();
                            (*MaterialSlots[i])->LoadResources();
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
            if (auto* StaticMeshChildActor = dynamic_cast<CStaticMesh*>(ChildStaticMeshRef))
            {
                // Replace mesh only if the child didn't override it
                if (StaticMeshChildActor->MeshResource == OldMesh)
                {
                    StaticMeshChildActor->MeshResource = MeshResource;
                }
            }
            ChildReference = ChildReference->Next;
        }
    }

    void CStaticMesh::HandleBaseAssetNumMaterialsChange(const bool& bAdded)
    {
        const u16 AffectedMaterialSlot = GetNumMaterialSlots() - 1;
        auto ChildReference = &ChildReferences.Head;
        while (ChildReference && ChildReference->Element)
        {
            auto* ChildStaticMeshRef = ChildReference->Element;
            if (auto* StaticMeshChildActor = dynamic_cast<CStaticMesh*>(ChildStaticMeshRef))
            {
                // Modify only if the child didn't override the mesh
                if (StaticMeshChildActor->MeshResource == MeshResource)
                {
                    if (bAdded)
                    {
                        StaticMeshChildActor->MaterialSlots.Add(nullptr);
                    }
                    else
                    {
                        delete StaticMeshChildActor->GetMaterialSlot(AffectedMaterialSlot);
                        StaticMeshChildActor->MaterialSlots.RemoveLast();
                    }
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
            if (auto* StaticMeshChildActor = dynamic_cast<CStaticMesh*>(ChildStaticMeshRef))
            {
                if (StaticMeshChildActor->MeshResource == MeshResource)
                {
                    auto* BaseAssetMaterial = *MaterialSlots[InMaterialSlot];
                    auto* ChildMaterialSlot = StaticMeshChildActor->GetMaterialSlot(InMaterialSlot);
                    if (!ChildMaterialSlot)
                    {
                        StaticMeshChildActor->AddMaterial(BaseAssetMaterial ? BaseAssetMaterial->GetCopy() : nullptr);
                    }
                    else if (ChildMaterialSlot->GetID() == InOldMaterial->GetID())
                    {
                        delete *StaticMeshChildActor->MaterialSlots[InMaterialSlot];
                        *StaticMeshChildActor->MaterialSlots[InMaterialSlot] = (*MaterialSlots[InMaterialSlot])->GetCopy();
                    }
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
                OutDescription.MeshResourceId.Value    = MeshResource->GetID();
            }

            for (u16 i = 0; i < BaseStaticMesh->MaterialSlots.GetLength(); ++i)
            {
                if (i < MaterialSlots.GetLength() && *MaterialSlots[i] && *MaterialSlots[i] != *(BaseStaticMesh->MaterialSlots[i]))
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
            OutDescription.MeshResourceId.Value    = MeshResource ? MeshResource->GetID() : sole::INVALID_UUID;
            OutDescription.MeshResourceId.bChanged = true;
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                OutDescription.MaterialIds.push_back({ *MaterialSlots[i] ? (*MaterialSlots[i])->GetID() : sole::INVALID_UUID, true });
            }
        }

        OutDescription.Id              = Id;
        OutDescription.ParentId        = Parent ? Parent->Id : 0;
        OutDescription.Name            = Name;
        OutDescription.Postion         = VecToFloat3(Transform.Translation);
        OutDescription.Rotation        = QuatToFloat4(Transform.Rotation);
        OutDescription.Scale           = VecToFloat3(Transform.Scale);
        OutDescription.bVisible        = bVisible;
        OutDescription.bReverseNormals = bReverseNormals;
    }

    void CStaticMesh::InternalSaveToResourceFile(const FString& InFilePath)
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

    CStaticMesh* CStaticMesh::CreateActor(CStaticMesh* BaseActorResource, CWorld* InWorld, const FStaticMeshDescription& InStaticMeshDescription)
    {
        assert(BaseActorResource);
        assert(InWorld);

        auto* Parent = InStaticMeshDescription.ParentId == 0 ? nullptr : InWorld->GetActorById(InStaticMeshDescription.ParentId);

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
        StaticMesh->Id   = InStaticMeshDescription.Id;
        StaticMesh->Transform.Translation = Float3ToVec(InStaticMeshDescription.Postion);
        StaticMesh->Transform.Rotation    = Float4ToQuat(InStaticMeshDescription.Rotation);
        StaticMesh->Transform.Scale       = Float3ToVec(InStaticMeshDescription.Scale);
        StaticMesh->bVisible              = InStaticMeshDescription.bVisible;
        StaticMesh->SetReverseNormals(InStaticMeshDescription.bReverseNormals);
        StaticMesh->BaseActorAsset = BaseActorResource;
        StaticMesh->BaseStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorResource);

        for (u16 i = 0; i < BaseActorResource->GetNumMaterialSlots(); ++i)
        {
            if (i < InStaticMeshDescription.MaterialIds.size() && InStaticMeshDescription.MaterialIds[i].bChanged)
            {
                StaticMesh->AddMaterial(GEngine.GetMaterialsHolder().Get(InStaticMeshDescription.MaterialIds[i].Value)->GetCopy());
                StaticMesh->GetMaterialSlot(i)->LoadResources();
            }
            else
            {
                StaticMesh->AddMaterial(BaseActorResource->GetMaterialSlot(i)->GetCopy());
            }
        }

        InWorld->AddStaticMesh(StaticMesh);
        BaseActorResource->AddChildReference(StaticMesh);

        return StaticMesh;
    }

    void CStaticMesh::UpdateMaterialSlots(CStaticMesh const* BaseStaticMesh)
    {
        if (BaseStaticMesh)
        {
            for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                auto* Material = GetMaterialSlot(i);
                if (Material)
                {
                    if (Material != BaseStaticMesh->GetMaterialSlot(i))
                    {
                        Material->UnloadResources();
                    }
                    delete *MaterialSlots[i];
                }
            }

            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{ BaseStaticMesh->GetNumMaterialSlots(), true };

            for (int i = 0; i < BaseStaticMesh->GetNumMaterialSlots(); ++i)
            {
                MaterialSlots.Add(BaseStaticMesh->GetMaterialSlot(i)->GetCopy());
            }
        }
        else
        {
            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{ 1, true };
        }
    }

    IActor* CStaticMesh::CreateActorAsset(const FDString& InName) const
    {
        auto* ActorAsset          = new CStaticMesh{ InName, nullptr, nullptr, MeshResource, EStaticMeshType::STATIONARY };
        ActorAsset->MaterialSlots = FArray<CMaterial*>(MaterialSlots.GetLength(), true);
        for (u16 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            ActorAsset->MaterialSlots.Add(GEngine.GetMaterialsHolder().Get(GetMaterialSlot(i)->GetID()));
        }
        return ActorAsset;
    }

    CStaticMesh* CStaticMesh::CreateEmptyActorAsset(const FDString& InName)
    {
        auto* EmptyActorAsset          = new CStaticMesh{ InName, nullptr, nullptr, nullptr, EStaticMeshType::STATIONARY };
        EmptyActorAsset->MaterialSlots = FArray<CMaterial*>(1, true);
        return EmptyActorAsset;
    }

    void CStaticMesh::LoadAsset()
    {
        IActor::LoadAsset();
        if (bAssetLoaded)
        {
            return;
        }

        LUCID_LOG(ELogLevel::INFO, "Loading StaticMesh %s", *Name);

        assert(ResourcePath.GetLength());

        // We might already loaded this asset previously and then unloaded it
        // in that case we already have a mesh resource and it's material, but their data might not be loaded
        if (MeshResource)
        {
            MeshResource->Acquire(false, true);
            for (u32 i = 0; i < MaterialSlots.GetLength(); ++i)
            {
                GetMaterialSlot(i)->LoadResources();
            }
            bAssetLoaded = true;
            return;
        }

        FStaticMeshDescription StaticMeshDescription;
        if (ReadFromJSONFile(StaticMeshDescription, *ResourcePath)) // @TODO strings from here don't get freed
        {
            MeshResource = GEngine.GetMeshesHolder().Get(StaticMeshDescription.MeshResourceId.Value);
            MeshResource->Acquire(false, true);

            for (u16 i = 0; i < StaticMeshDescription.MaterialIds.size(); ++i)
            {
                auto* Material = GEngine.GetMaterialsHolder().Get(StaticMeshDescription.MaterialIds[i].Value);
                MaterialSlots.Add(Material);
                Material->LoadResources();
            }

            bAssetLoaded = true;
        }
        else
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to load asset %s - couldn't read file %s", *Name, *ResourcePath)
        }
    }

    void CStaticMesh::UnloadAsset()
    {
        if (!bAssetLoaded)
        {
            return;
        }

        LUCID_LOG(ELogLevel::INFO, "Unloading StaticMesh %s", *Name);

        MeshResource->Release();

        for (u32 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            if (CMaterial* Material = GetMaterialSlot(i))
            {
                Material->UnloadResources();
            }
        }

        bAssetLoaded = false;
    }

    IActor* CStaticMesh::CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        LoadAsset();
        auto* SpawnedMesh = new CStaticMesh{ Name.GetCopy(), nullptr, InWorld, MeshResource, Type };
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            SpawnedMesh->AddMaterial((*MaterialSlots[i])->GetCopy());
        }
        SpawnedMesh->Transform.Translation = InSpawnPosition;
        SpawnedMesh->BaseActorAsset        = this;
        SpawnedMesh->BaseStaticMesh        = this;
        SpawnedMesh->MeshResource          = MeshResource;
        AddChildReference(SpawnedMesh);
        InWorld->AddStaticMesh(SpawnedMesh);
        return SpawnedMesh;
    }

    IActor* CStaticMesh::CreateCopy()
    {
        auto* SpawnedMesh = new CStaticMesh{ Name.GetCopy(), Parent, World, MeshResource, Type };
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            SpawnedMesh->AddMaterial((*MaterialSlots[i])->GetCopy());
        }
        SpawnedMesh->Transform = Transform;
        SpawnedMesh->Transform.Translation += glm::vec3{ 1, 0, 0 };
        SpawnedMesh->BaseActorAsset = BaseActorAsset;
        SpawnedMesh->BaseStaticMesh = BaseStaticMesh;
        BaseStaticMesh->AddChildReference(SpawnedMesh);
        World->AddStaticMesh(SpawnedMesh);
        return SpawnedMesh;
    }

    void CStaticMesh::OnAddToWorld(CWorld* InWorld)
    {
        IActor::OnAddToWorld(InWorld);
        World->AddStaticMesh(this);

        MaterialSlots = FArray<CMaterial*>(BaseStaticMesh->GetNumMaterialSlots());
        for (u8 i = 0; i < BaseStaticMesh->GetNumMaterialSlots(); ++i)
        {
            MaterialSlots.Add(BaseStaticMesh->GetMaterialSlot(i)->GetCopy());
        }
    }

    void CStaticMesh::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        IActor::OnRemoveFromWorld(InbHardRemove);
        World->RemoveStaticMesh(Id);

        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            if (auto* MaterialSlot = *MaterialSlots[i])
            {
                if (MaterialSlot)
                {
                    if (MaterialSlot != BaseStaticMesh->GetMaterialSlot(i))
                    {
                        MaterialSlot->UnloadResources();
                    }
                    delete MaterialSlot;
                }
            }
        }

        MaterialSlots.Free();
    }
} // namespace lucid::scene
