#include "scene/renderable/mesh_renderable.hpp"

namespace lucid::scene
{
    CStaticMesh::CStaticMesh(const FDString& InName,
                             const IRenderable* InParent,
                             gpu::CVertexArray* InVertexArray,
                             CMaterial* InMaterial,
                             const EStaticMeshType& InType)
    : IRenderable(InName, InParent), VertexArray(InVertexArray), Material(InMaterial), Type(InType)
    {
    }
} // namespace lucid::scene
