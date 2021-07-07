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
        OutDescription.Name                  = Name;
        OutDescription.ResolutionX           = TerrainSettings.Resolution.x;
        OutDescription.ResolutionZ           = TerrainSettings.Resolution.y;
        OutDescription.TerrainMeshResourceId = TerrainMesh ? TerrainMesh->GetID() : sole::INVALID_UUID;
        OutDescription.TerrainMaterialId     = TerrainMaterial ? TerrainMaterial->GetID() : sole::INVALID_UUID;
    }

    lucid::resources::CMeshResource* CreateFlatTerrainMesh(const FTerrainSettings& TerrainSettings)
    {
        const u32 VertexCount  = (TerrainSettings.Resolution.x + 1) * (TerrainSettings.Resolution.y + 1);
        const u32 IndicesCount = TerrainSettings.Resolution.x * TerrainSettings.Resolution.y * 6;

        const u32 TerrainVertexDataSize  = VertexCount * sizeof(FTerrainVertex);
        const u32 TerrainIndicesDataSize = IndicesCount * sizeof(u32);

        FMemBuffer VertexDataBuffer  = CreateMemBuffer(TerrainVertexDataSize);
        FMemBuffer IndicesDataBuffer = CreateMemBuffer(TerrainIndicesDataSize);

        FTerrainVertex* VertexData  = (FTerrainVertex*)VertexDataBuffer.Pointer;
        u32*            IndicesData = (u32*)IndicesDataBuffer.Pointer;

        const glm::vec3 UpperLeft = { -TerrainSettings.GridSize.x / 2, 0, -TerrainSettings.GridSize.y / 2 };

        const glm::vec2 CellSize = TerrainSettings.GridSize / glm::vec2(TerrainSettings.Resolution);
        const glm::vec2 UVStep   = { 1.f / TerrainSettings.Resolution.x, 1 / TerrainSettings.Resolution.y };

        // Generate vertex data
        for (u32 z = 0; z < TerrainSettings.Resolution.y + 1; ++z)
        {
            for (u32 x = 0; x < TerrainSettings.Resolution.x + 1; ++x)
            {
                // Vertex data
                VertexData->Position = UpperLeft;
                VertexData->Position += glm::vec3{ x * CellSize.x, 0, z * CellSize.y };

                VertexData->Position.y = (1 - (glm::length(VertexData->Position) /
                                               glm::length(glm::vec3{ TerrainSettings.GridSize.x / 2.f, 0, TerrainSettings.GridSize.y / 2.f })));
                VertexData->Position.y = VertexData->Position.y * VertexData->Position.y * VertexData->Position.y *
                                         (VertexData->Position.y * (VertexData->Position.y * 6 - 15) + 10);
                VertexData->Position.y *= 5;

                VertexData->Normal = glm::vec3{ 0 };
                VertexData->TextureCoords += glm::vec2{ x * UVStep.x, z * UVStep.y };

                VertexData += 1;
            }
        }

        // Reset the pointer so we can index when generating normals
        VertexData = (FTerrainVertex*)VertexDataBuffer.Pointer;

        const auto StoreIndicesAndUpdateNormals = [VertexData,
                                                   &IndicesData](const u32& FaceVert0, const u32& FaceVert1, const u32& FaceVert2) -> void {
            *IndicesData = FaceVert0;
            IndicesData += 1;

            *IndicesData = FaceVert1;
            IndicesData += 1;

            *IndicesData = FaceVert2;
            IndicesData += 1;

            // Normals
            const glm::vec3 Edge0 = VertexData[FaceVert0].Position - VertexData[FaceVert1].Position;
            const glm::vec3 Edge1 = VertexData[FaceVert2].Position - VertexData[FaceVert1].Position;

            const glm::vec3 Normal = glm::cross(Edge0, Edge1);

            VertexData[FaceVert0].Normal += Normal;
            VertexData[FaceVert1].Normal += Normal;
            VertexData[FaceVert2].Normal += Normal;
        };

        // Generate indices, calculate normals
        for (u32 z = 0; z < TerrainSettings.Resolution.y; ++z)
        {
            for (u32 x = 0; x < TerrainSettings.Resolution.x; ++x)
            {
                // First triangle
                {
                    const u32 FaceVert0 = (z * (TerrainSettings.Resolution.x + 1)) + x;
                    const u32 FaceVert1 = (z * (TerrainSettings.Resolution.x + 1)) + x + 1;
                    const u32 FaceVert2 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x;

                    StoreIndicesAndUpdateNormals(FaceVert0, FaceVert1, FaceVert2);
                }

                // Second triangle
                {
                    const u32 FaceVert0 = (z * (TerrainSettings.Resolution.x + 1)) + x + 1;
                    const u32 FaceVert1 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x + 1;
                    const u32 FaceVert2 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x;

                    StoreIndicesAndUpdateNormals(FaceVert0, FaceVert1, FaceVert2);
                }
            }
        }

        // Normalize the normals
        for (u32 i = 0; i < (TerrainSettings.Resolution.x + 1) * (TerrainSettings.Resolution.y + 1); ++i)
        {
            VertexData[i].Normal = glm::normalize(VertexData[i].Normal);
        }

        VertexDataBuffer.Size  = TerrainVertexDataSize;
        IndicesDataBuffer.Size = TerrainIndicesDataSize;

        auto* TerrainMesh = new resources::CMeshResource{ sole::uuid4(),
                                                          "Terrain",
                                                          SPrintf("assets/meshes/Terrain_%s.asset", sole::uuid4().str().c_str()),
                                                          0,
                                                          TerrainVertexDataSize + TerrainIndicesDataSize,
                                                          resources::MESH_SERIALIZATION_VERSION };

        GEngine.AddMeshResource(TerrainMesh);

        resources::FSubMesh TerrainSubMesh;
        TerrainSubMesh.bHasPositions     = true;
        TerrainSubMesh.bHasNormals       = true;
        TerrainSubMesh.bHasTangetns      = false;
        TerrainSubMesh.bHasUVs           = true;
        TerrainSubMesh.VertexDataBuffer  = VertexDataBuffer;
        TerrainSubMesh.ElementDataBuffer = IndicesDataBuffer;
        TerrainSubMesh.VertexCount       = VertexCount;
        TerrainSubMesh.ElementCount      = IndicesCount;
        TerrainSubMesh.MaterialIndex     = 0;
        TerrainMesh->SubMeshes.Add(TerrainSubMesh);

        TerrainMesh->MinPosX = UpperLeft.x;
        TerrainMesh->MaxPosX = -UpperLeft.x;

        TerrainMesh->MaxPosY  = 0;
        TerrainMesh->MinPosY  = 0;
        TerrainMesh->DrawMode = gpu::EDrawMode::TRIANGLES;

        TerrainMesh->SaveSynchronously();

        VertexDataBuffer.Free();
        IndicesDataBuffer.Free();

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
        DefaultTerrainSettings.Resolution = { 100, 100 };
        DefaultTerrainSettings.GridSize   = { 100, 100 };

        resources::CMeshResource* TerrainMesh = CreateFlatTerrainMesh(DefaultTerrainSettings);

        auto* TerrainActorAsset = new CTerrain{ InName, nullptr, nullptr, DefaultTerrainSettings, TerrainMesh, GEngine.GetDefaultMaterial() };

        TerrainActorAsset->ResourceId   = sole::uuid4();
        TerrainActorAsset->ResourcePath = SPrintf("assets/actors/%s.asset", *TerrainActorAsset->Name);

        TerrainActorAsset->SaveToResourceFile();

        return TerrainActorAsset;
    }

} // namespace lucid::scene
