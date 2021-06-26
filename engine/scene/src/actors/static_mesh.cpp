﻿#include "scene/actors/static_mesh.hpp"

#include "resources/mesh_resource.hpp"
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
                    OldBaseMeshAsset = OldStaticMesh;
                }

                if (auto* NewStaticMesh = dynamic_cast<CStaticMesh*>(BaseActorAsset))
                {
                    NewMeshResource  = NewStaticMesh->MeshResource;
                    NewBaseMeshAsset = NewStaticMesh;
                }

                GEngine.AddActorWithDirtyResources(this);
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
                            OldMeshResource      = MeshResource;
                            NewMeshResource      = BaseStaticMesh->MeshResource;
                            bUpdateMaterialSlots = true;
                            GEngine.AddActorWithDirtyResources(this);
                        }
                    }
                }
                resources::CMeshResource* OldMesh = MeshResource;
                ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);
                if (OldMesh != MeshResource)
                {
                    OldMeshResource              = OldMesh;
                    NewMeshResource              = MeshResource;
                    bPropagateMeshResourceChange = true;
                    GEngine.AddActorWithDirtyResources(this);
                }
            }
            else
            {
                // Actor asset editing
                resources::CMeshResource* OldMesh = MeshResource;
                ImGui::Text("Mesh resource:");
                ImGuiMeshResourcePicker("static_mesh_mesh", &MeshResource);

                if (OldMesh != MeshResource)
                {
                    OldMeshResource              = OldMesh;
                    NewMeshResource              = MeshResource;
                    bPropagateMeshResourceChange = true;
                    GEngine.AddActorWithDirtyResources(this);
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
                    MaterialsToUnload.push_back(LastMaterial);

                    if (!BaseActorAsset)
                    {
                        HandleBaseAssetNumMaterialsChange(false);
                    }
                    else if (LastMaterial != BaseStaticMesh->GetMaterialSlot(MaterialSlot))
                    {
                        MaterialsToDelete.push_back(LastMaterial);
                    }

                    GEngine.AddActorWithDirtyResources(this);
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
                            HandleInstanceMaterialSlotChange(OldMaterial, i);
                        }
                        else
                        {

                            HandleBaseAssetMaterialSlotChange(OldMaterial, i);
                        }

                        GEngine.AddActorWithDirtyResources(this);
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
        auto      ChildReference       = &ChildReferences.Head;
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
        MaterialsToUnload.push_back(InOldMaterial);
        MaterialsToLoad.push_back(*MaterialSlots[InMaterialSlot]);
        
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

        OutDescription.Id              = ActorId;
        OutDescription.ParentId        = Parent ? Parent->ActorId : 0;
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
        MeshResource->Acquire(false, true);

        auto* StaticMesh    = new CStaticMesh{ InStaticMeshDescription.Name, Parent, InWorld, MeshResource, InStaticMeshDescription.Type };
        StaticMesh->ActorId = InStaticMeshDescription.Id;
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
            }
            else
            {
                StaticMesh->AddMaterial(BaseActorResource->GetMaterialSlot(i)->GetCopy());
            }
            StaticMesh->GetMaterialSlot(i)->LoadResources();
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
                    Material->UnloadResources();
                    delete *MaterialSlots[i];
                }
            }

            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{ BaseStaticMesh->GetNumMaterialSlots(), true };

            for (int i = 0; i < BaseStaticMesh->GetNumMaterialSlots(); ++i)
            {
                if (CMaterial* NewMaterial = BaseStaticMesh->GetMaterialSlot(i))
                {
                    NewMaterial->LoadResources();
                    MaterialSlots.Add(NewMaterial->GetCopy());
                }
            }
        }
        else
        {
            MaterialSlots.Free();
            MaterialSlots = FArray<CMaterial*>{ 1, true };
        }
    }

    void CStaticMesh::HandleInstanceMaterialSlotChange(CMaterial* InOldMaterial, const u16& InMaterialIndex)
    {
        // Create an instance of the material for this mseh
        CMaterial* NewMaterial = *MaterialSlots[InMaterialIndex];
        NewMaterial            = NewMaterial->GetCopy();
        SetMaterialSlot(InMaterialIndex, NewMaterial);

        if (InOldMaterial)
        {
            // Check if material at this slot wasn't previously changed by this instance -
            if (BaseStaticMesh->GetMaterialSlot(InMaterialIndex) != InOldMaterial)
            {
                // if it was - free it, as the base asset has a different material at this slot
                MaterialsToUnload.push_back(InOldMaterial);
            }
            MaterialsToDelete.push_back(InOldMaterial);

            // Check if we can recycle the old material's index
            if (NewMaterial->GetType() == InOldMaterial->GetType())
            {
                NewMaterial->MaterialBufferIndex = InOldMaterial->MaterialBufferIndex;
            }
            else
            {
                NewMaterial->TypeToFree                = InOldMaterial->GetType();
                NewMaterial->MaterialBufferIndexToFree = InOldMaterial->MaterialBufferIndex;
            }
        }

        MaterialsToLoad.push_back(NewMaterial);
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

    CStaticMesh* CStaticMesh::LoadActorAsset(const FStaticMeshDescription& InStaticMeshDescription)
    {
        auto* ActorAsset         = new CStaticMesh{ InStaticMeshDescription.Name, nullptr, nullptr, nullptr, EStaticMeshType::STATIONARY };
        ActorAsset->MeshResource = GEngine.GetMeshesHolder().Get(InStaticMeshDescription.MeshResourceId.Value);

        ActorAsset->MaterialSlots = FArray<CMaterial*>(InStaticMeshDescription.MaterialIds.size(), true);
        for (u16 i = 0; i < InStaticMeshDescription.MaterialIds.size(); ++i)
        {
            if (InStaticMeshDescription.MaterialIds[i].Value == sole::INVALID_UUID)
            {
                ActorAsset->MaterialSlots.Add(nullptr);
            }
            else
            {
                ActorAsset->MaterialSlots.Add(GEngine.GetMaterialsHolder().Get(InStaticMeshDescription.MaterialIds[i].Value));
            }
        }

        return ActorAsset;
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

        MeshResource->Acquire(false, true);

        for (u16 i = 0; i < GetNumMaterialSlots(); ++i)
        {
            GetMaterialSlot(i)->LoadResources();
        }

        bAssetLoaded = true;
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
            if (*MaterialSlots[i])
            {
                SpawnedMesh->AddMaterial((*MaterialSlots[i])->GetCopy());
                SpawnedMesh->GetMaterialSlot(i)->LoadResources();
            }
            else
            {
                SpawnedMesh->AddMaterial(nullptr);
            }
        }
        SpawnedMesh->Transform.Translation = InSpawnPosition;
        SpawnedMesh->BaseActorAsset        = this;
        SpawnedMesh->BaseStaticMesh        = this;
        SpawnedMesh->MeshResource          = MeshResource;
        AddChildReference(SpawnedMesh);
        InWorld->AddStaticMesh(SpawnedMesh);
        SpawnedMesh->MeshResource->Acquire(false, true);
        return SpawnedMesh;
    }

    IActor* CStaticMesh::CreateCopy()
    {
        auto* SpawnedMesh = new CStaticMesh{ Name.GetCopy(), Parent, World, MeshResource, Type };
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            if (*MaterialSlots[i])
            {
                SpawnedMesh->AddMaterial((*MaterialSlots[i])->GetCopy());
                SpawnedMesh->GetMaterialSlot(i)->LoadResources();
            }
            else
            {
                SpawnedMesh->AddMaterial(nullptr);
            }
        }
        SpawnedMesh->Transform = Transform;
        SpawnedMesh->Transform.Translation += glm::vec3{ 1, 0, 0 };
        SpawnedMesh->BaseActorAsset = BaseActorAsset;
        SpawnedMesh->BaseStaticMesh = BaseStaticMesh;
        BaseStaticMesh->AddChildReference(SpawnedMesh);
        World->AddStaticMesh(SpawnedMesh);
        SpawnedMesh->MeshResource->Acquire(false, true);
        return SpawnedMesh;
    }

    void CStaticMesh::OnAddToWorld(CWorld* InWorld)
    {
        IActor::OnAddToWorld(InWorld);
        World->AddStaticMesh(this);
    }

    void CStaticMesh::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        IActor::OnRemoveFromWorld(InbHardRemove);
        World->RemoveStaticMesh(ActorId);

        if (InbHardRemove)
        {
            CleanupAfterRemove();
        }
    }

    void CStaticMesh::CleanupAfterRemove()
    {
        for (u8 i = 0; i < MaterialSlots.GetLength(); ++i)
        {
            if (auto* MaterialSlot = *MaterialSlots[i])
            {
                if (MaterialSlot)
                {
                    MaterialSlot->UnloadResources();
                    delete MaterialSlot;
                }
            }
        }

        MeshResource->Release();
        MaterialSlots.Free();
    }

    void CStaticMesh::UpdateDirtyResources()
    {
        if (OldMeshResource)
        {
            OldMeshResource->Release();
            OldMeshResource = nullptr;
        }

        if (NewMeshResource)
        {
            NewMeshResource->Acquire(false, true);
            NewMeshResource = nullptr;
        }

        if (bPropagateMeshResourceChange)
        {
            HandleBaseAssetMeshResourceChange(OldMeshResource);
            bPropagateMeshResourceChange = false;
        }

        if (OldBaseMeshAsset)
        {
            OldBaseMeshAsset->RemoveChildReference(this);
            OldBaseMeshAsset = nullptr;
        }

        if (NewBaseMeshAsset)
        {
            MeshResource = NewBaseMeshAsset->MeshResource;
            NewBaseMeshAsset->AddChildReference(this);
            BaseStaticMesh = NewBaseMeshAsset;
            UpdateMaterialSlots(NewBaseMeshAsset);
            NewBaseMeshAsset = nullptr;
        }

        if (bUpdateMaterialSlots)
        {
            UpdateMaterialSlots(BaseStaticMesh);
            bUpdateMaterialSlots = false;
        }

        for (CMaterial* Material : MaterialsToLoad)
        {
            Material->LoadResources();
        }

        for (CMaterial* Material : MaterialsToUnload)
        {
            Material->UnloadResources();
        }

        for (CMaterial* Material : MaterialsToDelete)
        {
            delete Material;
        }

        MaterialsToLoad.clear();
        MaterialsToUnload.clear();
        MaterialsToDelete.clear();
    };
} // namespace lucid::scene
