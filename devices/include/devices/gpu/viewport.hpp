#pragma once

#include "common/types.hpp"

namespace lucid::gpu
{
    struct FViewport
    {
        u32 X, Y;
        u32 Width, Height;
    };

    void SetViewport(const FViewport& ViewportToUse);
} // namespace lucid::gpu