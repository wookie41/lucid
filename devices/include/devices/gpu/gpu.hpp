#pragma once

#include <cstdint>
#include "common/types.hpp"

namespace lucid::gpu
{
    /////////////// Buffers ///////////////

    enum ClearableBuffers : uint8_t
    {
        COLOR = 1,
        DEPTH = 2,
        ACCUMULATION = 4,
        STENCIL = 8
    };

    void ClearBuffers(const ClearableBuffers& BuffersToClear);
    void SetClearColor(const color& Color);

    /////////////// Depth tests ///////////////

    void EnableDepthTest();
    void DisableDepthTest();
    
    /////////////// Viewport ///////////////

    struct Viewport
    {
        uint32_t X, Y;
        uint32_t Width, Height;
    };

    void SetViewprot(const Viewport& ViewportToUse);
} // namespace lucid::gpu
