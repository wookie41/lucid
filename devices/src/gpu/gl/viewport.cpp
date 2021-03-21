#include "devices/gpu/viewport.hpp"

#include "GL/glew.h"

namespace lucid::gpu
{
    void SetViewport(const FViewport& ViewportToUse)
    {
        glViewport(ViewportToUse.X, ViewportToUse.Y, ViewportToUse.Width, ViewportToUse.Height);
    }
} // namespace lucid::gpu