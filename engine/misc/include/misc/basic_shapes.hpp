#pragma once

namespace lucid::gpu
{
    class CVertexArray;
    struct FGPUState;
}

namespace lucid::misc
{
    /** Creates a new VAO with data for screen-wide quad */
    gpu::CVertexArray* CreateQuadVAO();

    /** Creates a new VAO with data for a unit cube  quad */
    gpu::CVertexArray* CreateCubeVAO();

} // namespace lucid::graphics