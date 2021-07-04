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
                 resources::CMeshResource* InTerrainMesh,
                 CMaterial*                InTerrainMaterial);

        /** Terrain stuff */
        static EActorType  GetActorTypeStatic() { return EActorType::TERRAIN; }
        virtual EActorType GetActorType() const override { return EActorType::TERRAIN; }

        inline resources::CMeshResource* GetTerrainMesh() const { return TerrainMesh; }
        inline CMaterial*                GetTerrainMaterial() const { return TerrainMaterial; }

        /** Actor interface stuff */

        float GetVerticalMidPoint() const override;

        void FillDescription(FTerrainDescription& OutDescription) const;

        static CTerrain* CreateAsset(const FDString& InName);
        virtual IActor*  CreateAssetFromActor(const FDString& InName) const override;
        static CTerrain* LoadAsset(const FTerrainDescription& InTerrainDescription);

        virtual IActor* LoadActor(CWorld* InWorld, FActorEntry const* InActorDescription);
        virtual IActor* CreateActorInstanceFromAsset(CWorld* InWorld, const glm::vec3& InSpawnPosition) override;
        virtual IActor* CreateActorCopy() override;

        /** Asset stuff */
        virtual void LoadAssetResources() override;
        virtual void UnloadAssetResources() override;
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
        resources::CMeshResource* TerrainMesh     = nullptr;
        CMaterial*                TerrainMaterial = nullptr;
    };

    CTerrain* CreateTerrainAsset(const FDString& InName);
} // namespace lucid::scene
