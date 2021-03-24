#pragma once

#include <cstdint>
#include "common/strings.hpp"
#include "glm/glm.hpp"
#include "scene/transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace lucid::gpu
{
    class CVertexArray;
} // namespace lucid::gpu

namespace lucid::scene
{
    enum class ERenderableType : u8
    {
        STATIC,
        DYNAMIC
    };

    class CMaterial;

    struct FRenderable
    {
        explicit FRenderable(const FDString& InName) : Name(InName) {}
        FRenderable(const FDString& InName, const FRenderable& rhs) : Name(InName)
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

        FDString Name;
        ERenderableType Type;
        FTransform3D Transform;
        gpu::CVertexArray* VertexArray;
        scene::CMaterial* Material;

        bool bReverseNormals = false;
    };
} // namespace lucid::scene
