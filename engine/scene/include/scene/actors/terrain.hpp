#pragma once

#include "scene/actors/actor.hpp"
#include "schemas/types.hpp"

namespace lucid
{
    namespace resources
    {
        class CMeshResource;
    } // namespace resources

    namespace gpu
    {
        class CVertexArray;
        class CGPUBuffer;
    } // namespace gpu
} // namespace lucid

namespace lucid::scene
{
    struct FTerrainSettings
    {
        /** Size of the grid in world space*/
        glm::vec2 GridSize = { 0, 0 };

        /** Resolution of the terrain mesh, i.e. how many cells in x and y does it have */
        glm::ivec2 Resolution = { 0, 0 };
    };

    struct FTerrainVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TextureCoords;
    };

    class CTerrain : public IActor
    {
      public:
        CTerrain(const FDString&           InName,
                 IActor*                   InParent,
                 CWorld*                   InWorld,
                 const FTerrainSettings&   InTerrainSettings,
                 resources::CMeshResource* InTerrainMesh);

        /** Terrain stuff */
        static EActorType  GetActorTypeStatic() { return EActorType::TERRAIN; }
        virtual EActorType GetActorType() const override { return EActorType::TERRAIN; }

        inline resources::CMeshResource* GetTerrainMesh() const { return TerrainMesh; }

        /** Actor interface stuff */
        void FillDescription(FTerrainDescription& OutDescription) const;

        virtual IActor* CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition) override;
        virtual IActor* CreateCopy() override;
        virtual IActor* CreateActorAsset(const FDString& InName) const override;

        static CTerrain* CreateActor(CTerrain* BaseActorResource, CWorld* InWorld, const FTerrainDescription& InTerrainDescription);
        static CTerrain* LoadActorAsset(const FTerrainDescription& InTerrainDescription);

        /** Asset stuff */
        virtual void LoadAsset() override;
        virtual void UnloadAsset() override;
        virtual void UpdateDirtyResources() override;

        /** World callbacks */
        virtual void OnAddToWorld(CWorld* InWorld) override;
        virtual void OnRemoveFromWorld(const bool& InbHardRemove) override;
        virtual void CleanupAfterRemove() override;

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

      protected:
        virtual void InternalSaveToResourceFile(const FString& InFilePath) override;
#endif

      protected:
        FTerrainSettings          TerrainSettings;
        resources::CMeshResource* TerrainMesh = nullptr;
    };
} // namespace lucid::scene
