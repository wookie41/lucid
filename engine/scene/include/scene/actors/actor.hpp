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
    class CCamera;

    /** Base interface for all things that can be a part of the scene and thus can be rendered */
    class IActor
    {
      public:
        IActor(const FDString& InName, IActor* InParent, CWorld* InWorld) : Name(InName), Parent(InParent), World(InWorld)
        {
            if (Parent)
            {
                Parent->AddChild(this);
            }
        }
        IActor(const FDString& InName, const IActor& InRHS) : Name(InName)
        {
            Parent    = InRHS.Parent;
            Transform = InRHS.Transform;
            World     = InRHS.World;
        }

        glm::mat4 CalculateModelMatrix()
        {
            glm::mat4  Identity{ 1 };
            const auto Translation = glm::translate(Identity, Transform.Translation);
            const auto Rotation    = glm::mat4_cast(Transform.Rotation);
            const auto Scale       = glm::scale(Identity, Transform.Scale);

            const glm::mat4 ModelMatrix = Translation * Rotation * Scale;

            CachedModelMatrix = ModelMatrix;
            return Parent ? Parent->CalculateModelMatrix() * ModelMatrix : ModelMatrix;
        }

#if DEVELOPMENT
        /** Editor stuff */
        virtual void    UIDrawActorDetails();
        virtual IActor* UIDrawHierarchy();
        virtual void    DrawGizmos(scene::CCamera const* InCamera);

        /**
         *  InActorResourceName will be the name of the resource file to which this actor is saved
         *  If empty, then it means there is already a resource file (under ResourcePath) for this actor present and we simply override it
         *  If not empty, then a new resource file is create
         */
        void SaveToResourceFile();
        virtual void UpdateDirtyResources() {};

    protected:
        virtual void InternalSaveToResourceFile(const FString& InActorResourceName) = 0;

      public:
#endif

        virtual EActorType GetActorType() const = 0;

        virtual float GetVerticalMidPoint() const = 0;
        virtual float GetMaxY() const { return 0; }
        virtual float GetMaxZ() const { return 0; }

        virtual IActor* CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition) = 0;
        virtual IActor* CreateCopy()                                                           = 0;

        /** Creates an actor asset from actocr instance so it can be saved to an asset file */
        virtual IActor* CreateActorAsset(const FDString& InName) const = 0;

        /** Used when actor asset is referenced for the first time, up to this point no data is loaded for an asset */
        virtual void LoadAsset(){};
        virtual void UnloadAsset() {}

        virtual void OnAddToWorld(CWorld* InWorld);

        /**
         * If HardRemove = true, then this call with also free memory occupied by child actors
         * Settings this flag to false is useful in the editor when deleting an actor, because we might want to undo the deletion
         * in which case we don't want to free.
         * If we're sure that we want to permanently delete the actor an it's children, ten call CleanupAfterRemove()
         */
        virtual void OnRemoveFromWorld(const bool& InbHardRemove);

        virtual void CleanupAfterRemove();

        inline void AddChild(IActor* InChild)
        {
            Children.Add(InChild);
            InChild->Parent = this;
        }
        inline void RemoveChild(IActor* InChild)
        {
            Children.Remove(InChild);
            InChild->Parent = nullptr;
        }

        void AddChildReference(IActor* InChildReference);
        void RemoveChildReference(IActor* InChildReference);

        virtual ~IActor() = default;

        /**
         * Unique id for an actor in the world, used e.x. by the renderer when generating the hitmap texture
         * Starts with 1, 0 = INVALID
         */
        u32          ActorId     = 0;
        IActor*      Parent = nullptr;
        FDString     Name;
        FTransform3D Transform;
        bool         bVisible   = true;
        UUID         ResourceId = sole::INVALID_UUID;
        FDString     ResourcePath{ "" };
        CWorld*      World; // World that this actor is in
        glm::mat4    CachedModelMatrix{ 1 };

        FLinkedList<IActor> Children;

        /** Actors that specify this actor as their BaseActorAsset, used for propagating changes */
        FLinkedList<IActor> ChildReferences;

        bool bAssetLoaded = false;

        IActor* BaseActorAsset     = nullptr;
        IActor* PrevBaseActorAsset = nullptr;

      protected:
        bool bMovable    = true;
        bool bParentable = true;

#if DEVELOPMENT
        bool bRenaming = false;
#endif
    };
} // namespace lucid::scene
