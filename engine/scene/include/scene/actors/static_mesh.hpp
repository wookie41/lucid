#pragma once

#include "scene/actors/actor.hpp"
#include "scene/actors/actor_enums.hpp"
#include "schemas/types.hpp"
#include "common/collections.hpp"
#include "resources/mesh_resource.hpp"

namespace lucid::resources
{
    class CMeshResource;
}

namespace lucid::scene
{

    class CStaticMesh : public IActor
    {
      public:
        CStaticMesh(const FDString&           InName,
                    IActor*                   InParent,
                    CWorld*                   InWorld,
                    resources::CMeshResource* InMeshResource,
                    const EStaticMeshType&    InType,
                    const math::FAABB& InAABB);

        inline void SetReverseNormals(const bool& InReverseNormals) { bReverseNormals = InReverseNormals; }
        inline bool GetReverseNormals() const { return bReverseNormals; }

        inline EStaticMeshType           GetMeshType() const { return Type; }
        inline resources::CMeshResource* GetMeshResource() const { return MeshResource; }
        CMaterial*                       GetMaterialSlot(const u16& InMaterialSlotIndex) const { return *MaterialSlots[InMaterialSlotIndex]; }
        void                             AddMaterial(CMaterial* InMaterial) { MaterialSlots.Add(InMaterial); }

        void SetMaterialSlot(const u16& InMaterialSlotIndex, CMaterial* InMaterial) const { (*MaterialSlots[InMaterialSlotIndex]) = InMaterial; }
        u32  GetNumMaterialSlots() const { return MaterialSlots.GetLength(); }
        
        static EActorType  GetActorTypeStatic() { return EActorType::STATIC_MESH; }
        virtual EActorType GetActorType() const override { return EActorType::STATIC_MESH; }

        virtual IActor*    CreateActorInstanceFromAsset(CWorld* InWorld, const glm::vec3& InSpawnPosition) override;
        virtual IActor*    LoadActor(CWorld* InWorld, FActorEntry const* InActorDescription) override;
        virtual IActor*    CreateActorCopy() override;

        static CStaticMesh* CreateAsset(const FDString& InName);
        virtual IActor*     CreateAssetFromActor(const FDString& InName) const override;
        static CStaticMesh* LoadAsset(const FStaticMeshDescription& InStaticMeshDescription);
        
        virtual void LoadAssetResources() override;
        virtual void UnloadAssetResources() override;

        virtual void OnAddToWorld(CWorld* InWorld) override;
        virtual void OnRemoveFromWorld(const bool& InbHardRemove) override;
        virtual void CleanupAfterRemove() override;

        virtual void UpdateDirtyResources() override;

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

      protected:
        virtual void InternalSaveAssetToFile(const FString& InFilePath) override;
        void         UpdateMaterialSlots(CStaticMesh const* BaseStaticMesh);
        void         HandleInstanceMaterialSlotChange(CMaterial* InOldMaterial, const u16& InMaterialIndex);
        void         HandleBaseAssetMeshResourceChange(resources::CMeshResource* OldMesh);
        void         HandleBaseAssetNumMaterialsChange(const bool& bAdded);
        void         HandleBaseAssetMaterialSlotChange(CMaterial* InOldMaterial, const u8& InMaterialSlot);

        CStaticMesh* OldBaseMeshAsset = nullptr;
        CStaticMesh* NewBaseMeshAsset = nullptr;

        bool bPropagateMeshResourceChange = false;
        bool bUpdateMaterialSlots         = false;

        resources::CMeshResource* OldMeshResource = nullptr;
        resources::CMeshResource* NewMeshResource = nullptr;

        std::vector<CMaterial*> MaterialsToLoad;
        std::vector<CMaterial*> MaterialsToUnload;
        std::vector<CMaterial*> MaterialsToDelete;

      public:
#endif

        void FillDescription(FStaticMeshDescription& OutDescription) const;

        EStaticMeshType           Type;
        resources::CMeshResource* MeshResource    = nullptr;
        bool                      bReverseNormals = false;

        FArray<CMaterial*> MaterialSlots{ 1, true };

        CStaticMesh* BaseStaticMesh = nullptr;
    };

} // namespace lucid::scene
