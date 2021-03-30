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
    class CMaterial;

    /** Base interface for all things that can be a part of the scene and thus can be rendered */
    struct IRenderable
    {
        IRenderable(const FDString& InName, const IRenderable* InParent) : Name(InName), Parent(InParent) {}
        IRenderable(const FDString& InName, const IRenderable& InRHS) : Name(InName)
        {
            Parent      =   InRHS.Parent;
            Transform   =   InRHS.Transform;
        }

        glm::mat4 CalculateModelMatrix() const
        {
            glm::mat4 Identity{ 1 };
            const auto Translation  = glm::translate(Identity, Transform.Translation);
            const auto Rotation     = glm::mat4_cast(Transform.Rotation);
            const auto Scale        = glm::scale(Identity, Transform.Scale);
            const glm::mat4 ModelMatrix = Translation * Rotation * Scale;
            return Parent ? Parent->CalculateModelMatrix() * ModelMatrix : ModelMatrix;
        }

        const IRenderable*      Parent     = nullptr;
        const FDString          Name;
        FTransform3D            Transform;
    };
} // namespace lucid::scene
