#pragma once

#include "common/strings.hpp"
#include "common/collections.hpp"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include "scene/transform.hpp"
#include "scene/actors/actor_enums.hpp"
namespace lucid::gpu
{
    class CVertexArray;
} // namespace lucid::gpu

namespace lucid::scene
{
    class CMaterial;
    class CWorld;
    
    /** Base interface for all things that can be a part of the scene and thus can be rendered */
    class IActor
    {
    public:
        IActor(const FDString& InName, IActor* InParent, CWorld* InWorld) : Name(InName), Parent(InParent), World(InWorld)
        {
            if (Parent)
            {
                Parent->Children.Add(this);
            }
        }
        IActor(const FDString& InName, const IActor& InRHS) : Name(InName)
        {
            Parent      =   InRHS.Parent;
            Transform   =   InRHS.Transform;
            World       =   InRHS.World;
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

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails();

        /**
         *  InActorResourceName will be the name of the resource file to which this actor is saved
         *  If empty, then it means there is already a resource file (under ResourcePath) for this actor present and we simply override it
         *  If not empty, then a new resource file is create
         */
        void            SaveToResourceFile();
    protected:
        virtual void    InternalSaveToResourceFile(const FString& InActorResourceName) = 0;
    public:
#endif

        virtual EActorType  GetActorType() const = 0;
        virtual float       GetVerticalMidPoint() const = 0;

        /** Creates an actor asset from actocr instance so it can be saved to an asset file */
        virtual IActor*     CreateActorAsset(const FDString& InName) const = 0;

        /** Used when actor asset is referenced for the first time, up to this point no data is loaded for an asset */
        virtual void        LoadAsset() = 0;

        /**
         * Unique id for an actor, used e.x. by the renderer when generating the hitmap texture
         * Starts with 1, 0 = INVALID
         */
        u32                     Id = 0; 
        IActor*                 Parent     = nullptr;
        const FDString          Name;
        FTransform3D            Transform;
        bool                    bVisible = true;
        UUID                    ResourceId = sole::INVALID_UUID;
        FDString                ResourcePath { "" };
        CWorld*                 World; // World that this actor is in
        FArray<IActor*>         Children { 1, true };
        bool                    bAssetLoaded = false;

    protected:

        IActor*                 BaseActorAsset = nullptr;
        bool                    bBaseActorAssetChanged = false;
        
        virtual ~IActor() = default;    


    };
} // namespace lucid::scene
