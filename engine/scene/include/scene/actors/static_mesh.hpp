#pragma once
#include "scene/actors/actor.hpp"

namespace lucid::resources
{
    class CMeshResource;
}

namespace lucid::scene
{
    enum EStaticMeshType
    {
        STATIONARY,
        MOVABLE
    };

    class CStaticMesh : public IActor
    {
      public:
        CStaticMesh(const FDString& InName,
                    const IActor* InParent,
                    resources::CMeshResource* InMeshResource,
                    CMaterial* InMaterial,
                    const EStaticMeshType& InType);

        inline resources::CMeshResource*    GetMeshResource() const { return MeshResource; }
        inline CMaterial*                   GetMaterial() const { return Material; }

        inline void SetReverseNormals(const bool& InReverseNormals) { bReverseNormals = InReverseNormals; }
        inline bool GetReverseNormals() const { return bReverseNormals; }

        virtual float GetVerticalMidPoint() const override;

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;
#endif
    
      protected:
        EStaticMeshType             Type;
        CMaterial*                  Material = nullptr;
        resources::CMeshResource*   MeshResource = nullptr;
        bool                        bReverseNormals = false;
    };
} // namespace lucid::scene
