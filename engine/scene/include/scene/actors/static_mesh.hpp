#pragma once

#include "scene/actors/actor.hpp"
#include "scene/actors/actor_enums.hpp"
#include "schemas/types.hpp"

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

        static  EActorType   GetActorTypeStatic() { return EActorType::STATIC_MESH; };
        virtual EActorType   GetActorType() const override { return EActorType::STATIC_MESH; };
        static  CStaticMesh* CreateActor(CStaticMesh const* BaseActorResource, CWorld* InWorld, const FStaticMeshDescription& InStaticMeshDescription);
        
#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;
    protected:
        virtual void _SaveToResourceFile(const FString& InFilePath) override;
    public:
#endif

        void FillDescription(FStaticMeshDescription& OutDescription) const;
        
        EStaticMeshType             Type;
        resources::CMeshResource*   MeshResource = nullptr;
        bool                        bReverseNormals = false;

        FArray<CMaterial*>          MaterialSlots { 1, true };

        CStaticMesh const*          BaseStaticMesh = nullptr;
    };
} // namespace lucid::scene
