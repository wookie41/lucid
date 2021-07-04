#include "scene/actors/terrain.hpp"
#include "scene/world.hpp"

#include "engine/engine.hpp"

#include "schemas/json.hpp"

#include "devices/gpu/vao.hpp"
#include "devices/gpu/buffer.hpp"

#include "resources/mesh_resource.hpp"
#include "resources/serialization_versions.hpp"

#include <cassert>

namespace lucid::scene
{
    CTerrain::CTerrain(const FDString&           InName,
                       IActor*                   InParent,
                       CWorld*                   InWorld,
                       const FTerrainSettings&   InTerrainSettings,
                       resources::CMeshResource* InTerrainMesh,
                       CMaterial*                InTerrainMaterial)
    : IActor(InName, InParent, InWorld), TerrainSettings(InTerrainSettings), TerrainMesh(InTerrainMesh), TerrainMaterial(InTerrainMaterial)
    {
    }

    float CTerrain::GetVerticalMidPoint() const { return 0; }

    void CTerrain::FillDescription(FTerrainDescription& OutDescription) const
    {
        OutDescription.ResolutionX           = TerrainSettings.Resolution.x;
        OutDescription.ResolutionZ           = TerrainSettings.Resolution.y;
        OutDescription.TerrainMeshResourceId = TerrainMesh ? TerrainMesh->GetID() : sole::INVALID_UUID;
        OutDescription.TerrainMaterialId     = TerrainMaterial ? TerrainMaterial->GetID() : sole::INVALID_UUID;
    }

    lucid::resources::CMeshResource* CreateFlatTerrainMesh(const FTerrainSettings& TerrainSettings)
    {
        const u32       TerrainVertexDataSize = TerrainSettings.Resolution.x * TerrainSettings.Resolution.y * 4 * sizeof(FTerrainVertex);
        FMemBuffer      VertexDataBuffer      = CreateMemBuffer(TerrainVertexDataSize);
        FTerrainVertex* VertexData            = (FTerrainVertex*)VertexDataBuffer.Pointer;

        const glm::vec3 UpperLeft = { -(TerrainSettings.GridSize.x / 2), 0, -TerrainSettings.GridSize.y / 2 };

        u8 HorizontalStep = 0;
        u8 VerticalStep   = 1;

        // Generate a triangle strip mesh for this
        const glm::vec2 CellSize = TerrainSettings.GridSize / glm::vec2(TerrainSettings.Resolution);
        const glm::vec2 UVStep   = { 1.f / TerrainSettings.Resolution.x, 1 / TerrainSettings.Resolution.y };

        for (u32 z = 0; z < TerrainSettings.Resolution.y; ++z)
        {
            for (u32 x = 0; x < TerrainSettings.Resolution.x * 4; ++x)
            {
                VertexData->Position =
                  UpperLeft + glm::vec3{ (HorizontalStep * CellSize.x) + (x * CellSize.x), 0, (VerticalStep * CellSize.y) + (z * CellSize.y) };
                VertexData->Normal        = { 0, 1, 0 };
                VertexData->TextureCoords = glm::vec2{ 0, 1 } + glm::vec2{ HorizontalStep * UVStep.x, (VerticalStep * UVStep.y) + (z * UVStep.y) };

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
        TerrainMesh->MaxPosX = -UpperLeft.x;

        TerrainMesh->MaxPosY  = 0;
        TerrainMesh->MinPosY  = 0;
        TerrainMesh->DrawMode = gpu::EDrawMode::TRIANGLE_STRIP;

        return TerrainMesh;
    }

    IActor* CTerrain::CreateActorInstanceFromAsset(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        auto* Terrain = new CTerrain{ CopyToString("Terrain"), nullptr, InWorld, TerrainSettings, TerrainMesh, TerrainMaterial->GetCopy() };
        AddAssetReference(Terrain);
        InWorld->AddTerrain(Terrain);
        return Terrain;
    }

    IActor* CTerrain::CreateActorCopy()
    {
        auto* TerrainActorCopy = new CTerrain{ Name.GetCopy(), Parent, World, TerrainSettings, TerrainMesh, TerrainMaterial };
        AddAssetReference(TerrainActorCopy);
        return TerrainActorCopy;
    }

    IActor* CTerrain::CreateAssetFromActor(const FDString& InName) const
    {
        return new CTerrain{ Name.GetCopy(), nullptr, nullptr, TerrainSettings, TerrainMesh, TerrainMaterial };
    }

    IActor* CTerrain::LoadActor(CWorld* InWorld, FActorEntry const* InActorDescription)
    {
        auto* TerrainDescription = (FTerrainDescription const*)InActorDescription;

        FTerrainSettings TerrainSettings;
        TerrainSettings.Resolution.x = TerrainDescription->ResolutionX;
        TerrainSettings.Resolution.y = TerrainDescription->ResolutionZ;
        TerrainSettings.GridSize.x   = TerrainDescription->GridSizeX;
        TerrainSettings.GridSize.y   = TerrainDescription->GridSizeZ;

        resources::CMeshResource* TerrainMeshResource = nullptr;
        if (TerrainDescription->TerrainMeshResourceId == sole::INVALID_UUID)
        {
            TerrainMeshResource = TerrainMesh;
        }
        else
        {
            TerrainMeshResource = GEngine.GetMeshesHolder().Get(TerrainDescription->TerrainMeshResourceId);
        }

        TerrainMeshResource->Acquire(false, true);

        auto* TerrainActor =
          new CTerrain{ TerrainDescription->Name, InWorld->GetActorById(TerrainDescription->ParentId), InWorld, TerrainSettings, TerrainMeshResource,
                        TerrainMaterial };

        AddAssetReference(TerrainActor);

        return TerrainActor;
    }

    CTerrain* CTerrain::LoadAsset(const FTerrainDescription& InTerrainDescription)
    {
        FTerrainSettings TerrainSettings;
        TerrainSettings.Resolution.x = InTerrainDescription.ResolutionX;
        TerrainSettings.Resolution.y = InTerrainDescription.ResolutionZ;
        TerrainSettings.GridSize.x   = InTerrainDescription.GridSizeX;
        TerrainSettings.GridSize.y   = InTerrainDescription.GridSizeZ;

        return new CTerrain{ InTerrainDescription.Name,
                             nullptr,
                             nullptr,
                             TerrainSettings,
                             GEngine.GetMeshesHolder().Get(InTerrainDescription.TerrainMeshResourceId),
                             GEngine.GetMaterialsHolder().Get(InTerrainDescription.TerrainMaterialId) };
    }

    void CTerrain::LoadAssetResources()
    {
        if (bAssetResourcesLoaded)
        {
            return;
        }

        if (TerrainMesh)
        {
            TerrainMesh->Acquire(false, true);
        }

        bAssetResourcesLoaded = true;
    }

    void CTerrain::UnloadAssetResources()
    {
        if (!bAssetResourcesLoaded)
        {
            return;
        }

        if (TerrainMesh)
        {
            TerrainMesh->Release();
        }

        bAssetResourcesLoaded = false;
    }

    void CTerrain::UpdateDirtyResources()
    {
        // noop
    }

    void CTerrain::OnAddToWorld(CWorld* InWorld) { InWorld->AddTerrain(this); }

    void CTerrain::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        IActor::OnRemoveFromWorld(InbHardRemove);
        World->RemoveTerrain(this);

        if (InbHardRemove)
        {
            CleanupAfterRemove();
        }
    }

    void CTerrain::CleanupAfterRemove()
    {
        if (TerrainMesh)
        {
            TerrainMesh->Release();
        }

        if (TerrainMaterial)
        {
            TerrainMaterial->UnloadResources();
            delete TerrainMaterial;
        }
    }

    void CTerrain::UIDrawActorDetails() { IActor::UIDrawActorDetails(); }

    void CTerrain::InternalSaveToResourceFile(const FString& InFilePath)
    {
        FTerrainDescription TerrainDescription;
        FillDescription(TerrainDescription);
        WriteToJSONFile(TerrainDescription, *InFilePath);
    }

    CTerrain* CTerrain::CreateAsset(const FDString& InName)
    {
        static FTerrainSettings DefaultTerrainSettings;
        DefaultTerrainSettings.Resolution = { 1024, 1024 };
        DefaultTerrainSettings.GridSize   = { 100, 100 };

        resources::CMeshResource* TerrainMesh = CreateFlatTerrainMesh(DefaultTerrainSettings);

        return new CTerrain{ InName, nullptr, nullptr, DefaultTerrainSettings, TerrainMesh, GEngine.GetDefaultMaterial() };
    }

} // namespace lucid::scene
