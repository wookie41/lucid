#pragma once

#include <cstdint>
#include "common/strings.hpp"
#include "glm/glm.hpp"
#include "scene/transform.hpp"
#include "glm/gtx/quaternion.hpp"

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
        Renderable(const DString& InName) : Name(InName) {}
        Renderable(const DString& InName, const Renderable& rhs) : Name(InName)
        {
            Type = rhs.Type;
            Transform = rhs.Transform;
            VertexArray = rhs.VertexArray;
            Material = rhs.Material;
        }

        glm::mat4 CalculateModelMatrix() const
        {
            glm::mat4 identity{ 1 };
            auto translation = glm::translate(identity, Transform.Translation);
            auto rotation = glm::mat4_cast(Transform.Rotation);
            auto scale = glm::scale(identity, Transform.Scale);
            return translation * rotation * scale;            
        }

        DString Name;
        RenderableType Type;
        Transform3D Transform;
        gpu::VertexArray* VertexArray;
        scene::Material* Material;

        bool bReverseNormals = false;
    };
} // namespace lucid::scene
