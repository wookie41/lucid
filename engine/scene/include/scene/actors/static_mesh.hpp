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
                    const IActor* InParent,
                    CWorld* InWorld,
                    resources::CMeshResource* InMeshResource,
                    CMaterial* InMaterial,
                    const EStaticMeshType& InType);

        inline void SetReverseNormals(const bool& InReverseNormals) { bReverseNormals = InReverseNormals; }
        inline bool GetReverseNormals() const { return bReverseNormals; }

        inline EStaticMeshType              GetMeshType() const { return Type; }
        inline CMaterial*                   GetMaterial() const { return Material; }
        inline resources::CMeshResource*    GetMeshResource() const { return MeshResource; }
        
        virtual float GetVerticalMidPoint() const override;

        static  EActorType   GetActorTypeStatic() { return EActorType::STATIC_MESH; };
        virtual EActorType   GetActorType() const override { return EActorType::STATIC_MESH; };
        static  CStaticMesh* CreateActor(CWorld* InWorld, const FStaticMeshDescription& InStaticMeshDescription);
        
#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;
    protected:
        virtual void _SaveToResourceFile(const FString& InFilePath) override;
    public:
#endif

        void FillDescription(FStaticMeshDescription& OutDescription) const;
        
      protected:
        EStaticMeshType             Type;
        CMaterial*                  Material = nullptr;
        resources::CMeshResource*   MeshResource = nullptr;
        bool                        bReverseNormals = false;
    };
} // namespace lucid::scene
