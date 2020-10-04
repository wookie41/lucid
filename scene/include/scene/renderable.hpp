#pragma once

#include <cstdint>
#include "common/strings.hpp"

namespace lucid::gpu
{
    class VertexArray;
} // namespace lucid::gpu

namespace lucid::scene
{
    // intented to help to renderer in making decistion which Renderables'
    // are visible and which should be culled
    enum class RenderableType : uint8_t
    {
        STATIC,
        DYNAMIC
    };

    class Material;

    struct Renderable
    {
        String              Name;
        RenderableType      Type;
        gpu::VertexArray*   VertexArray;
        Material*           Material;
    };
} // namespace lucid::scene
