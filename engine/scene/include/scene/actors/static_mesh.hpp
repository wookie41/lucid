#pragma once

#include "scene/actors/actor.hpp"
#include "scene/actors/actor_enums.hpp"
#include "schemas/types.hpp"
#include "common/collections.hpp"

namespace lucid::resources
{
    class CMeshResource;
}

namespace lucid::scene
{

    class CStaticMesh : public IActor
    {
      public:
        CStaticMesh(const FDString& InName,
                    IActor* InParent,
                    CWorld* InWorld,
                    resources::CMeshResource* InMeshResource,
                    const EStaticMeshType& InType);

        inline void SetReverseNormals(const bool& InReverseNormals) { bReverseNormals = InReverseNormals; }
        inline bool GetReverseNormals() const { return bReverseNormals; }

        inline EStaticMeshType              GetMeshType() const { return Type; }
        inline resources::CMeshResource*    GetMeshResource() const { return MeshResource; }
        CMaterial*                          GetMaterialSlot(const u16& InMaterialSlotIndex) const { return *MaterialSlots[InMaterialSlotIndex]; }
        void                                AddMaterial(CMaterial* InMaterial) { MaterialSlots.Add(InMaterial); }
        void                                SetMaterialSlot(const u16& InMaterialSlotIndex, CMaterial* InMaterial) { (*MaterialSlots[InMaterialSlotIndex]) = InMaterial; }
        u32                                 GetNumMaterialSlots() const { return MaterialSlots.GetLength(); }
        
        virtual float GetVerticalMidPoint() const override;
        
        virtual float GetMaxY() const override { return MeshResource->MaxPosZ * Transform.Scale.y; }
        virtual float GetMaxZ() const override { return MeshResource->MaxPosZ * Transform.Scale.z;}

        static  EActorType      GetActorTypeStatic() { return EActorType::STATIC_MESH; }
        virtual EActorType      GetActorType() const override { return EActorType::STATIC_MESH; }
        virtual IActor*         CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition) override;
        virtual IActor*         CreateCopy() override;

        static  CStaticMesh*    CreateActor(CStaticMesh* BaseActorResource, CWorld* InWorld, const FStaticMeshDescription& InStaticMeshDescription);

        /** Creates an empty actor asset that lazily loads it's resources when referenced for the first time */
        static  CStaticMesh*    CreateEmptyActorAsset(const FDString& InName);

        virtual IActor*         CreateActorAsset(const FDString& InName) const override;
        virtual void            LoadAsset() override;
        virtual void            UnloadAsset() override;

        virtual void            OnAddToWorld(CWorld* InWorld) override;
        virtual void            OnRemoveFromWorld(const bool& InbHardRemove) override;
        
#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

    protected:
        virtual void    InternalSaveToResourceFile(const FString& InFilePath) override;
        void            UpdateMaterialSlots(CStaticMesh const* BaseStaticMesh);
        void            HandleBaseAssetMeshResourceChange(resources::CMeshResource* OldMesh);
        void            HandleBaseAssetNumMaterialsChange(const bool& bAdded);
        void            HandleBaseAssetMaterialSlotChange(CMaterial* InOldMaterial, const u8& InMaterialSlot);
    
    public:
#endif

        void FillDescription(FStaticMeshDescription& OutDescription) const;
        
        EStaticMeshType             Type;
        resources::CMeshResource*   MeshResource = nullptr;
        bool                        bReverseNormals = false;

        FArray<CMaterial*>          MaterialSlots { 1, true };

        CStaticMesh*                BaseStaticMesh = nullptr;
    };
} // namespace lucid::scene
