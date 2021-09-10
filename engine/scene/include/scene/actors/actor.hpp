#pragma once

#include "common/strings.hpp"
#include "common/collections.hpp"

#include "glm/glm.hpp"

#include "scene/transform.hpp"
#include "scene/actors/actor_enums.hpp"

#include "schemas/types.hpp"

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
        IActor(const FDString& InName, IActor* InParent, CWorld* InWorld, const math::FAABB& InAABB)
        : Name(InName), Parent(InParent), World(InWorld), AABB(InAABB)
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

        /** Saves the actor to a file.
         * If called on an actor asset, it just saves te asset.
         * If called on actor instance, it creates an asset based on the instance and saves it
         */
        void SaveAssetToFile();

        /**
         * Called by the engine before a frame so the actor can react to changes in it's resources
         * E.x. a mesh is changed on runtime and we have to unload the new mesh and load a new one
         */
        virtual void UpdateDirtyResources(){};

        /**
         * Called on the selected actor by the editor after the simulation is updated but before a frame.
         * Used to do editor-only stuff like drawing lines indicating light direction.
         */
        virtual void OnSelectedPreFrameRender();

      protected:
        virtual void InternalSaveAssetToFile(const FString& InActorResourceName) = 0;

      public:
#endif

        virtual EActorType GetActorType() const = 0;

        /** Called on the asset when to create an instance of this asset to place in the world */
        virtual IActor* CreateActorInstanceFromAsset(CWorld* InWorld, const glm::vec3& InSpawnPosition) = 0;

        /**
         *  Called on the asset when loading a world from file.
         *  The InActor and InActorDescription will be derived types needed to spawn an actor of a given type
         */
        virtual IActor* LoadActor(CWorld* InWorld, FActorEntry const* InActorDescription) = 0;

        /** Creates an actor asset from actor instance so it can be saved to an asset file */
        virtual IActor* CreateAssetFromActor(const FDString& InName) const = 0;
        virtual IActor* CreateActorCopy()                                  = 0;

        /** Used when actor asset is referenced for the first time, up to this point no data is loaded for an asset */
        virtual void LoadAssetResources() {}
        virtual void UnloadAssetResources() {}

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

        /**
         * Methods used to track actors in the current world that reference the asset
         * When the number of references drops to 0, resources used by the asset are unloaded
         */
        void AddAssetReference(IActor* InChildReference);
        void RemoveAssetReference(IActor* InChildReference);

        inline const math::FAABB& GetAABB() const { return AABB; }

        virtual void Tick(const float& InDeltaTime);

        virtual void OnScaled(const glm::vec3& InOldScale, const glm::vec3& InNewScale);
        virtual void OnTranslated(const glm::vec3& InOldPostion, const glm::vec3& InNewPosition);
        virtual void OnRotated(const glm::quat& InOldRotation, const glm::quat& InNewRotation);

        inline glm::vec3 GetActorForwardVector() const { return ForwardVector; }

        inline const FTransform3D& GetTransform() const { return Transform; }
        inline void                SetTransform(const FTransform3D& InTransform)
        {
            OldTransform     = Transform;
            Transform        = InTransform;
            bRotationUpdated = bTranslationUpdated = bRotationUpdated = true;
        }

        inline void Translate(const glm::vec3 InTranslation)
        {
            OldTransform = Transform;
            Transform.Translation += InTranslation;
            bTranslationUpdated = true;
        }

        inline void SetTranslation(const glm::vec3 InTranslation)
        {
            OldTransform          = Transform;
            Transform.Translation = InTranslation;
            bTranslationUpdated   = true;
        }

        void DrawAABB() const;

        virtual ~IActor() = default;

        /**
         * Unique id for an actor in the world, used e.x. by the renderer when generating the hitmap texture
         * Starts with 1, 0 = INVALID
         */
        u32       ActorId = 0;
        IActor*   Parent  = nullptr;
        FDString  Name;
        bool      bVisible = true;
        UUID      AssetId  = sole::INVALID_UUID;
        FDString  AssetPath{ "" };
        CWorld*   World; // World that this actor is in
        glm::mat4 CachedModelMatrix{ 1 };

        FLinkedList<IActor> Children;

        /** Actors that specify this asset as their BaseActorAsset, used for propagating changes */
        FLinkedList<IActor> AssetReferences;

        bool bAssetResourcesLoaded = false;

        IActor* BaseActorAsset     = nullptr;
        IActor* PrevBaseActorAsset = nullptr;

      protected:
        math::FAABB AABB;

        bool bMovable    = true;
        bool bParentable = true;

      private:
        FTransform3D Transform;
        FTransform3D OldTransform;

        glm::vec3 ForwardVector;

        bool bTranslationUpdated = false;
        bool bScaleUpdated       = false;
        bool bRotationUpdated    = false;

#if DEVELOPMENT
        bool bRenaming = false;
        bool bDrawAABB = false;
#endif
    };
} // namespace lucid::scene
