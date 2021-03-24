#pragma once

namespace lucid::gpu
{
    class CVertexArray;
    struct FGPUState;
}

namespace lucid::misc
{

    extern gpu::CVertexArray* QuadVertexArray;
    extern gpu::CVertexArray* CubeVertexArray;

    void InitBasicShapes(gpu::FGPUState* InGPUState);
} // namespace lucid::graphics