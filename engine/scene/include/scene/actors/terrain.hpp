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
        class CFence;
    } // namespace gpu
} // namespace lucid

namespace lucid::scene
{
    struct FTerrainSettings
    {
        /** Size of the grid in world space*/
        glm::vec2 GridSize = { 0, 0 };

        /** Resolution of the terrain mesh, i.e. how many cells in x and y does it have */
        glm::vec2 Resolution = { 0, 0 };

        bool bFlatMesh = true;

        i32 Seed = -1;

        i32   Octaves                  = 4;
        float Frequency                = 0.005f;
        float Amplitude                = 1.0f;
        float Lacunarity               = 4.152f;
        float Persistence              = 0.122f;
        float MinHeight                = 5.f;
        float MaxHeight                = 5.f;
        bool  bRegeneratingTerrainMesh = false;
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
                 CMaterial*                InTerrainMaterial,
                 const math::FAABB&        InAABB);

        /** Terrain stuff */
        static EActorType  GetActorTypeStatic() { return EActorType::TERRAIN; }
        virtual EActorType GetActorType() const override { return EActorType::TERRAIN; }

        inline resources::CMeshResource* GetTerrainMesh() const { return TerrainMesh; }
        inline CMaterial*                GetTerrainMaterial() const { return TerrainMaterial; }

        /** Actor interface stuff */

        void FillActorAssetDescription(FTerrainDescription& OutDescription) const;

        static CTerrain* CreateAsset(const FDString& InName, const FTerrainSettings& InTerrainSettings);
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
        void UpdateBaseAssetTerrainMeshUpdate(resources::CMeshResource* InNewTerrainMesh, const FTerrainSettings& InNewTerrainSettings);
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

      protected:
        virtual void InternalSaveAssetToFile(const FString& InFilePath) override;
#endif

      protected:
#if DEVELOPMENT

        FTerrainSettings NewTerrainSettings;
        bool             bSculpting                          = false;
        bool             bShouldFreeMainMemoryAfterSculpting = false;
        bool             bMaterialEditorOpen                 = false;

        float SculptStrength = 1;
        int   BrushSize      = 10;

      public:
        static constexpr u8 NumSculptingVBOs                     = 2;
        gpu::CGPUBuffer*    SculptingVBOs[NumSculptingVBOs]      = { nullptr };
        gpu::CFence*        SculptingVBOFences[NumSculptingVBOs] = { nullptr };
        u8                  CurrentSculptingVBOIndex             = 0;
        FTerrainVertex*     TerrainSculptData                    = nullptr;
        bool                bSculptFlushNeeded                   = false;
        bool                bSculpFlushed                        = false;
        gpu::CGPUBuffer*    OriginalTerrainVBO;

#endif
      protected:
        FTerrainSettings          TerrainSettings;
        resources::CMeshResource* TerrainMesh     = nullptr;
        CMaterial*                TerrainMaterial = nullptr;
    };

    CTerrain* CreateTerrainAsset(const FDString& InName);
} // namespace lucid::scene
