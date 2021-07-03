#include "scene/actors/terrain.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/buffer.hpp"
#include "resources/mesh_resource.hpp"
#include "resources/serialization_versions.hpp"

namespace lucid::scene
{
    CTerrain::CTerrain(const FDString&           InName,
                       IActor*                   InParent,
                       CWorld*                   InWorld,
                       const FTerrainSettings&   InTerrainSettings,
                       resources::CMeshResource* InTerrainMesh)
    : IActor(InName, InParent, InWorld), TerrainSettings(InTerrainSettings), TerrainMesh(InTerrainMesh)

    {
    }

    void CTerrain::FillDescription(FTerrainDescription& OutDescription) const
    {
        OutDescription.ResolutionX           = TerrainSettings.Resolution.x;
        OutDescription.ResolutionZ           = TerrainSettings.Resolution.y;
        OutDescription.TerrainMeshResourceId = TerrainMesh ? TerrainMesh->GetID() : sole::INVALID_UUID;
    }

    lucid::resources::CMeshResource* CreateFlatTerrainMesh(const FTerrainSettings& TerrainSettings)
    {
        const u32       TerrainVertexDataSize = TerrainSettings.Resolution.x * TerrainSettings.Resolution.y * sizeof(FTerrainVertex);
        FMemBuffer      VertexDataBuffer      = CreateMemBuffer(TerrainVertexDataSize);
        FTerrainVertex* VertexData            = (FTerrainVertex*)VertexDataBuffer.Pointer;

        const glm::vec3 UpperLeft = { -(TerrainSettings.GridSize.x / 2), 0, TerrainSettings.GridSize.y / 2 };

        u8 HorizontalStep = 0;
        u8 VerticalStep   = 1;

        // Generate a triangle strip mesh for this
        const glm::vec2 CellSize = TerrainSettings.GridSize / glm::vec2(TerrainSettings.Resolution);
        const glm::vec2 UVStep   = { 1.f / TerrainSettings.Resolution.x, 1 / TerrainSettings.Resolution.y };

        for (u32 z = 0; z < TerrainSettings.Resolution.y; ++z)
        {
            for (u32 x = 0; x < TerrainSettings.Resolution.x * 4; ++x)
            {
                VertexData->Position = UpperLeft + glm::vec3{ HorizontalStep * CellSize.x, 0, -((VerticalStep * CellSize.y) + (z * CellSize.y)) };
                VertexData->Normal   = { 0, 1, 0 };
                VertexData->TextureCoords = glm::vec2{ 0, 1 } + glm::vec2{ HorizontalStep * UVStep.x, -((VerticalStep * UVStep.y) + (z * UVStep.y)) };

                HorizontalStep = (HorizontalStep + 1) % 2;
                VerticalStep   = (VerticalStep + 1) % 2;

                VertexData += 1;
            }
        }
        VertexDataBuffer.Size = TerrainVertexDataSize;

        auto* TerrainMesh = new resources::CMeshResource{ sole::uuid4(),
                                                          "Terrain",
                                                          SPrintf("assets/meshes/Terrain_%s.asset", sole::uuid4().str().c_str()),
                                                          0,
                                                          TerrainVertexDataSize,
                                                          resources::MESH_SERIALIZATION_VERSION };

        resources::FSubMesh TerrainSubMesh;
        TerrainSubMesh.bHasPositions     = true;
        TerrainSubMesh.bHasNormals       = true;
        TerrainSubMesh.bHasTangetns      = false;
        TerrainSubMesh.bHasUVs           = true;
        TerrainSubMesh.VertexDataBuffer  = VertexDataBuffer;
        TerrainSubMesh.ElementDataBuffer = {};
        TerrainSubMesh.VertexCount       = TerrainSettings.Resolution.x * TerrainSettings.Resolution.y * 3;
        TerrainSubMesh.ElementCount      = 0;
        TerrainSubMesh.MaterialIndex     = 0;
        TerrainMesh->SubMeshes.Add(TerrainSubMesh);

        TerrainMesh->MinPosX = UpperLeft.x;
        TerrainMesh->MinPosY = -UpperLeft.y;

        TerrainMesh->MaxPosX = -UpperLeft.x;
        TerrainMesh->MaxPosY = UpperLeft.y;
    }

    IActor* CTerrain::CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        static FTerrainSettings DefaultTerrainSettings;
        DefaultTerrainSettings.Resolution = { 1024, 1024 };
        DefaultTerrainSettings.GridSize   = { 100, 100 };

        resources::CMeshResource* TerrainMesh = CreateFlatTerrainMesh(DefaultTerrainSettings);
    }
} // namespace lucid::scene
