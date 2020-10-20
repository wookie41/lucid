#pragma once

#include <cstdint>
#include "common/strings.hpp"
#include "glm/glm.hpp"
#include "scene/transform.hpp"

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
        DString Name;
        RenderableType Type;
        Transform3D Transform;
        gpu::VertexArray* VertexArray;
        scene::Material* Material;
    };
} // namespace lucid::scene
