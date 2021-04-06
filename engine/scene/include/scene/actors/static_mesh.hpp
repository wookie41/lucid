#pragma once
#include "scene/actors/actor.hpp"

namespace lucid::gpu
{
    class CVertexArray;
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
                    gpu::CVertexArray* InVertexArray,
                    CMaterial* InMaterial,
                    const EStaticMeshType& InType);

        inline gpu::CVertexArray* GetVertexArray() const { return VertexArray; }
        inline CMaterial* GetMaterial() const { return Material; }

        inline void SetReverseNormals(const bool& InReverseNormals) { bReverseNormals = InReverseNormals; }
        inline bool GetReverseNormals() const { return bReverseNormals; }

      protected:
        EStaticMeshType Type;
        CMaterial* Material = nullptr;
        gpu::CVertexArray* VertexArray = nullptr;
        bool bReverseNormals = false;
    };
} // namespace lucid::scene
