#pragma once

#include "common/types.hpp"

namespace lucid::gpu
{
    struct Viewport
    {
        u32 X, Y;
        u32 Width, Height;
    };

    void SetViewport(const Viewport& ViewportToUse);
} // namespace lucid::gpu