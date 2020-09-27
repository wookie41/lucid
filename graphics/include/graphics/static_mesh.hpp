#pragma once

namespace lucid::gpu
{
    class VertexArray;
}

namespace lucid::graphics
{


    struct StaticMesh
    {
        gpu::VertexArray* VertexArray = nullptr;
    };

    extern StaticMesh QuadShape;

    void InitBasicShapes();
    void DrawMesh(const StaticMesh const* Mesh);
} // namespace lucid::graphics