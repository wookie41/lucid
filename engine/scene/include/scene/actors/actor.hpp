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

    enum class EActorType : u8
    {
        STATIC_MESH,
        SKYBOX,
        LIGHT
    };
    
    /** Base interface for all things that can be a part of the scene and thus can be rendered */
    class IActor
    {
    public:
        IActor(const FDString& InName, const IActor* InParent) : Name(InName), Parent(InParent) {}
        IActor(const FDString& InName, const IActor& InRHS) : Name(InName)
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

        inline EActorType GetActorType() const { return ActorType; }

#if DEVELOPMENT

        /** Editor stuff */
        void EditorOnSelected();

#endif


        /**
         * Unique id for an actor, used e.x. by the renderer when generating the hitmap texture
         * Starts with 1, 0 = INVALID
         */
        u32                     Id = 0; 
        const IActor*           Parent     = nullptr;
        const FDString          Name;
        FTransform3D            Transform;

        virtual ~IActor() = default;
    
    protected:
        EActorType ActorType;
    };
} // namespace lucid::scene
