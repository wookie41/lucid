#pragma once

#include "common/types.hpp"

namespace lucid::gpu
{
    struct FViewport
    {
        u32 X = 0, Y = 0;
        u32 Width = 0, Height = 0;

        bool operator==(const FViewport& InRHS) { return X == InRHS.X && Y == InRHS.Y && Width == InRHS.Width && Height == InRHS.Height; }

        inline bool operator!=(const FViewport& InRHS) { return !(*this == InRHS); }
    };

    void SetViewport(const FViewport& ViewportToUse);
} // namespace lucid::gpu