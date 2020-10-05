#pragma once

namespace lucid::gpu
{
    class VertexArray;
}

namespace lucid::misc
{

    extern gpu::VertexArray* QuadVertexArray;
    extern gpu::VertexArray* CubeVertexArray;

    void InitBasicShapes();
} // namespace lucid::graphics