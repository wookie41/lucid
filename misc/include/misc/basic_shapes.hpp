#pragma once

namespace lucid::gpu
{
    class CVertexArray;
}

namespace lucid::misc
{

    extern gpu::CVertexArray* QuadVertexArray;
    extern gpu::CVertexArray* CubeVertexArray;

    void InitBasicShapes();
} // namespace lucid::graphics