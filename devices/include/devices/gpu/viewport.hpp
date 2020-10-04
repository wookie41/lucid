#pragma once

#include <cstdint>

namespace lucid::gpu
{
    struct Viewport
    {
        uint32_t X, Y;
        uint32_t Width, Height;
    };

    void SetViewport(const Viewport& ViewportToUse);
} // namespace lucid::gpu