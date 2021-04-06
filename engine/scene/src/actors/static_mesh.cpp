#include "scene/actors/static_mesh.hpp"

namespace lucid::scene
{
    CStaticMesh::CStaticMesh(const FDString& InName,
                             const IActor* InParent,
                             gpu::CVertexArray* InVertexArray,
                             CMaterial* InMaterial,
                             const EStaticMeshType& InType)
    : IActor(InName, InParent), VertexArray(InVertexArray), Material(InMaterial), Type(InType)
    {
        ActorType = EActorType::STATIC_MESH;
    }
} // namespace lucid::scene
