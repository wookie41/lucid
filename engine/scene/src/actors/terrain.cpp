#include "scene/actors/terrain.hpp"
#include "scene/world.hpp"
#include "scene/terrain_material.hpp"
#include "scene/renderer.hpp"

#include "engine/engine.hpp"

#include "schemas/json.hpp"

#include "devices/gpu/vao.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/fence.hpp"

#include "resources/mesh_resource.hpp"
#include "resources/serialization_versions.hpp"

#include "simplex_noise/simplex_noise.h"

#include "lucid_editor/imgui_lucid.h"

#include <random>
#include <glm/common.hpp>
#include <lucid_editor/editor.hpp>

#include "devices/gpu/texture_enums.hpp"
#include "resources/texture_resource.hpp"

namespace lucid::scene
{
    static constexpr gpu::EBufferMapPolicy UNSYNCHRONIZED_WRITE =
      (gpu::EBufferMapPolicy)(gpu::EBufferMapPolicy::BUFFER_WRITE | gpu::EBufferMapPolicy::BUFFER_UNSYNCHRONIZED);

    CTerrain::CTerrain(const FDString&           InName,
                       IActor*                   InParent,
                       CWorld*                   InWorld,
                       const FTerrainSettings&   InTerrainSettings,
                       resources::CMeshResource* InTerrainMesh,
                       CMaterial*                InTerrainMaterial,
                       const math::FAABB&        InAABB)
    : IActor(InName, InParent, InWorld, InAABB), TerrainSettings(InTerrainSettings), TerrainMesh(InTerrainMesh), TerrainMaterial(InTerrainMaterial),
      NewTerrainSettings(InTerrainSettings)
    {
    }

    void CTerrain::FillActorAssetDescription(FTerrainDescription& OutDescription) const
    {
        OutDescription.Name                  = Name;
        OutDescription.ResolutionX           = TerrainSettings.Resolution.x;
        OutDescription.ResolutionZ           = TerrainSettings.Resolution.y;
        OutDescription.GridSizeX             = TerrainSettings.GridSize.x;
        OutDescription.GridSizeZ             = TerrainSettings.GridSize.y;
        OutDescription.bFlat                 = TerrainSettings.bFlatMesh;
        OutDescription.Seed                  = TerrainSettings.Seed;
        OutDescription.Octaves               = TerrainSettings.Octaves;
        OutDescription.Frequency             = TerrainSettings.Frequency;
        OutDescription.Amplitude             = TerrainSettings.Amplitude;
        OutDescription.Lacunarity            = TerrainSettings.Lacunarity;
        OutDescription.Persistence           = TerrainSettings.Persistence;
        OutDescription.MinHeight             = TerrainSettings.MinHeight;
        OutDescription.MaxHeight             = TerrainSettings.MaxHeight;
        OutDescription.TerrainMeshResourceId = TerrainMesh ? TerrainMesh->GetID() : sole::INVALID_UUID;
        OutDescription.TerrainMaterialId     = TerrainMaterial ? TerrainMaterial->GetID() : sole::INVALID_UUID;
    }

    static lucid::resources::CMeshResource* GenerateTerrainMesh(const FTerrainSettings& TerrainSettings)
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

        const glm::vec2 CellSize = TerrainSettings.GridSize / TerrainSettings.Resolution;
        const glm::vec2 UVStep   = { 1.f / TerrainSettings.Resolution.x, 1 / TerrainSettings.Resolution.y };

        std::function<float(u32, u32)> HeightFunc = [](const u32& X, const u32& Z) -> float { return 0; };

        if (!TerrainSettings.bFlatMesh)
        {
            static std::default_random_engine RandomEngine{ std::mt19937(TerrainSettings.Seed == -1 ? math::RandomFloat() * UINT32_MAX :
                                                                                                      TerrainSettings.Seed) };

            static std::uniform_real_distribution<float> dis(0, 100000.0f);

            const auto SimplexGenerator =
              SimplexNoise{ TerrainSettings.Frequency, TerrainSettings.Amplitude, TerrainSettings.Lacunarity, TerrainSettings.Persistence };

            const float OffsetX = dis(RandomEngine);
            const float OffsetZ = dis(RandomEngine);

            HeightFunc = [&SimplexGenerator, &TerrainSettings, OffsetX, OffsetZ](const u32& X, const u32& Z) -> float {
                const float f = SimplexGenerator.fractal(TerrainSettings.Octaves, X + OffsetX, Z + OffsetZ);

                return math::Remap(f, -1, 1, TerrainSettings.MinHeight, TerrainSettings.MaxHeight);
            };
        }

        math::FAABB TerrainAABB;
        TerrainAABB.MinY = FLT_MAX;
        TerrainAABB.MaxY = 0;

        // Generate vertex data
        for (u32 z = 0; z < TerrainSettings.Resolution.y + 1; ++z)
        {
            for (u32 x = 0; x < TerrainSettings.Resolution.x + 1; ++x)
            {
                // Vertex data
                VertexData->Position = UpperLeft;
                VertexData->Position += glm::vec3{ x * CellSize.x, 0, z * CellSize.y };

                VertexData->Position.y = HeightFunc(x, z);

                if (VertexData->Position.y < TerrainAABB.MinY)
                {
                    TerrainAABB.MinY = VertexData->Position.y;
                }

                if (VertexData->Position.y > TerrainAABB.MaxY)
                {
                    TerrainAABB.MaxY = VertexData->Position.y;
                }

                VertexData->Normal = glm::vec3{ 0 };
                VertexData->TextureCoords += glm::vec2{ x * UVStep.x, z * UVStep.y };

                VertexData += 1;
            }
        }

        // Reset the pointer so we can index when generating normals
        VertexData = (FTerrainVertex*)VertexDataBuffer.Pointer;

        const auto StoreIndicesAndUpdateNormals = [VertexData, &IndicesData](const u32& FaceVert0, const u32& FaceVert1, const u32& FaceVert2) -> void {
            *IndicesData = FaceVert0;
            IndicesData += 1;

            *IndicesData = FaceVert1;
            IndicesData += 1;

            *IndicesData = FaceVert2;
            IndicesData += 1;

            // Normals
            const glm::vec3 Edge0 = VertexData[FaceVert0].Position - VertexData[FaceVert1].Position;
            const glm::vec3 Edge1 = VertexData[FaceVert1].Position - VertexData[FaceVert2].Position;

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
                    const u32 FaceVert0 = (z * (TerrainSettings.Resolution.x + 1)) + x + 1;
                    const u32 FaceVert1 = (z * (TerrainSettings.Resolution.x + 1)) + x;
                    const u32 FaceVert2 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x;

                    StoreIndicesAndUpdateNormals(FaceVert0, FaceVert1, FaceVert2);
                }

                // Second triangle
                {
                    const u32 FaceVert0 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x;
                    const u32 FaceVert1 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x + 1;
                    const u32 FaceVert2 = (z * (TerrainSettings.Resolution.x + 1)) + x + 1;

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

        TerrainAABB.MinX = UpperLeft.x;
        TerrainAABB.MaxX = -UpperLeft.x;

        TerrainAABB.MinZ = -UpperLeft.z;
        TerrainAABB.MaxZ = UpperLeft.z;

        auto* TerrainMesh = new resources::CMeshResource{ sole::uuid4(),
                                                          "Terrain",
                                                          SPrintf("assets/meshes/Terrain_%s.asset", sole::uuid4().str().c_str()),
                                                          0,
                                                          TerrainVertexDataSize + TerrainIndicesDataSize,
                                                          resources::MESH_SERIALIZATION_VERSION,
                                                          TerrainAABB };

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

        TerrainMesh->DrawMode = gpu::EDrawMode::TRIANGLES;

        TerrainMesh->SaveSynchronously();

        if (!TerrainSettings.bRegeneratingTerrainMesh)
        {
            VertexDataBuffer.Free();
            IndicesDataBuffer.Free();
        }

        return TerrainMesh;
    }

    IActor* CTerrain::CreateActorInstanceFromAsset(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        auto* Terrain = new CTerrain{ CopyToString("Terrain"), nullptr, InWorld, TerrainSettings, TerrainMesh, TerrainMaterial->GetCopy(), AABB };
        AddAssetReference(Terrain);
        InWorld->AddTerrain(Terrain);
        return Terrain;
    }

    IActor* CTerrain::CreateActorCopy()
    {
        auto* TerrainActorCopy = new CTerrain{ Name.GetCopy(), Parent, World, TerrainSettings, TerrainMesh, TerrainMaterial, AABB };
        AddAssetReference(TerrainActorCopy);
        return TerrainActorCopy;
    }

    IActor* CTerrain::CreateAssetFromActor(const FDString& InName) const
    {
        return new CTerrain{ Name.GetCopy(), nullptr, nullptr, TerrainSettings, TerrainMesh, TerrainMaterial, AABB };
    }

    IActor* CTerrain::LoadActor(CWorld* InWorld, FActorEntry const* InActorDescription)
    {
        auto* TerrainDescription = (FTerrainDescription const*)InActorDescription;

        resources::CMeshResource* TerrainMeshResource = nullptr;
        if (TerrainDescription->TerrainMeshResourceId == sole::INVALID_UUID)
        {
            TerrainMeshResource = TerrainMesh;
        }
        else
        {
            TerrainMeshResource = GEngine.GetMeshesHolder().Get(TerrainDescription->TerrainMeshResourceId);
        }

        auto* TerrainActor = new CTerrain{
            TerrainDescription->Name, InWorld->GetActorById(TerrainDescription->ParentId), InWorld, TerrainSettings, TerrainMeshResource, TerrainMaterial, AABB
        };

        AddAssetReference(TerrainActor);

        FTransform3D TerrainTransform;
        TerrainTransform.Translation = Float3ToVec(InActorDescription->Postion);
        TerrainTransform.Rotation    = Float4ToQuat(InActorDescription->Rotation);
        TerrainTransform.Scale       = Float3ToVec(InActorDescription->Scale);
        TerrainActor->SetTransform(TerrainTransform);

        InWorld->AddTerrain(TerrainActor);

        return TerrainActor;
    }

    CTerrain* CTerrain::LoadAsset(const FTerrainDescription& InTerrainDescription)
    {
        FTerrainSettings InTerrainSettings;
        InTerrainSettings.Resolution.x = InTerrainDescription.ResolutionX;
        InTerrainSettings.Resolution.y = InTerrainDescription.ResolutionZ;
        InTerrainSettings.GridSize.x   = InTerrainDescription.GridSizeX;
        InTerrainSettings.GridSize.y   = InTerrainDescription.GridSizeZ;
        InTerrainSettings.bFlatMesh    = InTerrainDescription.bFlat;
        InTerrainSettings.Seed         = InTerrainDescription.Seed;
        InTerrainSettings.Octaves      = InTerrainDescription.Octaves;
        InTerrainSettings.Frequency    = InTerrainDescription.Frequency;
        InTerrainSettings.Amplitude    = InTerrainDescription.Amplitude;
        InTerrainSettings.Lacunarity   = InTerrainDescription.Lacunarity;
        InTerrainSettings.Persistence  = InTerrainDescription.Persistence;
        InTerrainSettings.MinHeight    = InTerrainDescription.MinHeight;
        InTerrainSettings.MaxHeight    = InTerrainDescription.MaxHeight;

        resources::CMeshResource* TerrainMesh = GEngine.GetMeshesHolder().Get(InTerrainDescription.TerrainMeshResourceId);

        return new CTerrain{ InTerrainDescription.Name, nullptr,     nullptr,
                             InTerrainSettings,         TerrainMesh, GEngine.GetMaterialsHolder().Get(InTerrainDescription.TerrainMaterialId),
                             TerrainMesh->GetAABB() };
    }

    void CTerrain::LoadAssetResources()
    {
        IActor::LoadAssetResources();
        if (bAssetResourcesLoaded)
        {
            return;
        }

        if (TerrainMesh)
        {
            TerrainMesh->Acquire(false, true);
            AABB = TerrainMesh->GetAABB();
        }

        if (TerrainMaterial)
        {
            TerrainMaterial->LoadResources();
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
        IActor::CleanupAfterRemove();

        if (TerrainMaterial)
        {
            TerrainMaterial->UnloadResources();
            delete TerrainMaterial;
        }
    }

    void CTerrain::UpdateBaseAssetTerrainMeshUpdate(resources::CMeshResource* InNewTerrainMesh, const FTerrainSettings& InNewTerrainSettings)
    {
        auto ChildReference = &AssetReferences.Head;
        while (ChildReference && ChildReference->Element)
        {
            auto* ChildRef = ChildReference->Element;
            if (auto* TerrainRef = dynamic_cast<CTerrain*>(ChildRef))
            {
                InNewTerrainMesh->Acquire(TerrainRef->TerrainMesh->IsLoadedToMainMemory(), TerrainRef->TerrainMesh->IsLoadedToVideoMemory());
                TerrainRef->TerrainMesh->Release();

                TerrainRef->TerrainMesh     = InNewTerrainMesh;
                TerrainRef->TerrainSettings = TerrainRef->NewTerrainSettings = InNewTerrainSettings;
            }
            ChildReference = ChildReference->Next;
        }

        delete TerrainMesh;
        TerrainMesh = InNewTerrainMesh;

        TerrainSettings = NewTerrainSettings = InNewTerrainSettings;
        SaveAssetToFile();
    }

    void CTerrain::UIDrawActorDetails()
    {
        IActor::UIDrawActorDetails();

        if (ImGui::CollapsingHeader("Terrain"))
        {
            if (!bSculpting)
            {
                // Show generation panel if we aren't currently sculpting
                bool bRegenerateTerrain = false;
                bRegenerateTerrain |= ImGui::InputFloat2("Grid size", &NewTerrainSettings.GridSize[0]);
                bRegenerateTerrain |= ImGui::InputFloat2("Resolution", &NewTerrainSettings.Resolution[0]);
                bRegenerateTerrain |= ImGui::Checkbox("Flat", &NewTerrainSettings.bFlatMesh);
                bRegenerateTerrain |= ImGui::InputInt("Seed", &NewTerrainSettings.Seed);
                bRegenerateTerrain |= ImGui::InputInt("Octaves", &NewTerrainSettings.Octaves);
                bRegenerateTerrain |= ImGui::InputFloat("Frequency", &NewTerrainSettings.Frequency);
                bRegenerateTerrain |= ImGui::InputFloat("Amplitude", &NewTerrainSettings.Amplitude);
                bRegenerateTerrain |= ImGui::InputFloat("Lacunarity", &NewTerrainSettings.Lacunarity);
                bRegenerateTerrain |= ImGui::InputFloat("Persistence", &NewTerrainSettings.Persistence);
                bRegenerateTerrain |= ImGui::InputFloat("Min height", &NewTerrainSettings.MinHeight);
                bRegenerateTerrain |= ImGui::InputFloat("Max height", &NewTerrainSettings.MaxHeight);
                bRegenerateTerrain |= ImGui::Button("Regenerate");

                if (bRegenerateTerrain)
                {
                    NewTerrainSettings.bRegeneratingTerrainMesh = true;

                    resources::CMeshResource* NewTerrainMesh = GenerateTerrainMesh(NewTerrainSettings);
                    GEngine.RemoveMeshResource(TerrainMesh);
                    AABB = NewTerrainMesh->GetAABB();

                    if (auto* BaseTerrainAsset = dynamic_cast<CTerrain*>(BaseActorAsset))
                    {
                        BaseTerrainAsset->UpdateBaseAssetTerrainMeshUpdate(NewTerrainMesh, NewTerrainSettings);
                    }
                    else
                    {
                        UpdateBaseAssetTerrainMeshUpdate(NewTerrainMesh, NewTerrainSettings);
                    }
                }
            }

            // Handle sculpting
            if (bSculpting)
            {
                if (GetMouseWheelDelta() > 0)
                {
                    if (IsKeyPressed(SDLK_LSHIFT))
                    {
                        SculptStrength += 0.1;
                    }
                    else
                    {
                        BrushSize += 1;
                    }
                }
                else if (GetMouseWheelDelta() < 0)
                {
                    if (IsKeyPressed(SDLK_LSHIFT))
                    {
                        SculptStrength -= 0.1;
                    }
                    else
                    {
                        BrushSize -= 1;
                    }
                }

                BrushSize = BrushSize < 1 ? 1 : BrushSize;

                ImGui::Text("Brush size: %d", BrushSize);
                ImGui::Text("Brush strength: %f", SculptStrength);

                // Button to submit the new terrain
                if (ImGui::Button("Submit terrain"))
                {
                    // Recalculate the normals
                    for (u32 z = 0; z < TerrainSettings.Resolution.y; ++z)
                    {
                        for (u32 x = 0; x < TerrainSettings.Resolution.x; ++x)
                        {
                            // First triangle
                            {
                                const u32 FaceVert0 = (z * (TerrainSettings.Resolution.x + 1)) + x + 1;
                                const u32 FaceVert1 = (z * (TerrainSettings.Resolution.x + 1)) + x;
                                const u32 FaceVert2 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x;

                                // Normals
                                const glm::vec3 Edge0 = TerrainSculptData[FaceVert0].Position - TerrainSculptData[FaceVert1].Position;
                                const glm::vec3 Edge1 = TerrainSculptData[FaceVert1].Position - TerrainSculptData[FaceVert2].Position;

                                const glm::vec3 Normal = glm::cross(Edge0, Edge1);

                                TerrainSculptData[FaceVert0].Normal += Normal;
                                TerrainSculptData[FaceVert1].Normal += Normal;
                                TerrainSculptData[FaceVert2].Normal += Normal;
                            }

                            // Second triangle
                            {
                                const u32 FaceVert0 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x;
                                const u32 FaceVert1 = ((z + 1) * (TerrainSettings.Resolution.x + 1)) + x + 1;
                                const u32 FaceVert2 = (z * (TerrainSettings.Resolution.x + 1)) + x + 1;

                                // Normals
                                const glm::vec3 Edge0 = TerrainSculptData[FaceVert0].Position - TerrainSculptData[FaceVert1].Position;
                                const glm::vec3 Edge1 = TerrainSculptData[FaceVert1].Position - TerrainSculptData[FaceVert2].Position;

                                const glm::vec3 Normal = glm::cross(Edge0, Edge1);

                                TerrainSculptData[FaceVert0].Normal += Normal;
                                TerrainSculptData[FaceVert1].Normal += Normal;
                                TerrainSculptData[FaceVert2].Normal += Normal;
                            }
                        }
                    }

                    bSculpting                           = false;
                    GSceneEditorState.bBlockActorPicking = false;

                    // Save the changes
                    TerrainMesh->SaveSynchronously();

                    // Restore the original VBO so it gets freed properly
                    TerrainMesh->SubMeshes[0]->VAO->SetVertexBuffer(OriginalTerrainVBO);

                    // Load the edited terrain to the GPU
                    TerrainMesh->FreeVideoMemory();
                    TerrainMesh->LoadDataToVideoMemorySynchronously();

                    // Cleanup main memory if we need to
                    if (bShouldFreeMainMemoryAfterSculpting)
                    {
                        TerrainMesh->FreeMainMemory();
                        bShouldFreeMainMemoryAfterSculpting = false;
                    }

                    // Cleanup VBOs
                    for (u8 i = 0; i < NumSculptingVBOs; ++i)
                    {
                        SculptingVBOs[i]->Free();
                        delete SculptingVBOs[i];
                    }
                }

                float SculptDelta = 0;
                if (SceneWindow_GetActorUnderCursor() == this)
                {
                    if (IsMouseButtonPressed(EMouseButton::LEFT))
                    {
                        if (IsKeyPressed(SDLK_LSHIFT))
                        {
                            SculptDelta = -SculptStrength;
                        }
                        else
                        {
                            SculptDelta = SculptStrength;
                        }
                    }
                }

                // Sculpting
                if (SculptDelta != 0 && bSculpting)
                {
                    const float     DistanceToTerrain = SceneWindow_GetDistanceToCameraUnderCursor();
                    const glm::vec3 WorldRay          = GSceneEditorState.CurrentCamera->GetMouseRayInWorldSpace(GetMouseNDCPos(), DistanceToTerrain);

                    const glm::vec2 TerrainGridUpperLeft =
                      glm::vec2{ GetTransform().Translation.x, GetTransform().Translation.z } - (TerrainSettings.GridSize / 2.f);
                    const glm::vec2 RayProjectedToGrid = glm::vec2{ WorldRay.x, WorldRay.z } - TerrainGridUpperLeft;

                    int OffsetX = (RayProjectedToGrid.x / TerrainSettings.GridSize.x) * TerrainSettings.Resolution.x;
                    int OffsetZ = (RayProjectedToGrid.y / TerrainSettings.GridSize.y) * TerrainSettings.Resolution.y;

                    for (int z = -BrushSize; z <= BrushSize; ++z)
                    {
                        for (int x = -BrushSize; x <= BrushSize; ++x)
                        {
                            const int TerrainIndex = ((TerrainSettings.Resolution.x + 1) * (OffsetZ + z)) + OffsetX + x;
                            if (TerrainIndex < 0 || TerrainIndex > ((TerrainSettings.Resolution.x + 1) * (TerrainSettings.Resolution.y + 1) - 1))
                            {
                                continue;
                            }

                            TerrainSculptData[TerrainIndex].Position.y += SculptDelta;

                            if (TerrainSculptData[TerrainIndex].Position.y > AABB.MaxY)
                            {
                                AABB.MaxY = TerrainSculptData[TerrainIndex].Position.y;
                            }

                            if (TerrainSculptData[TerrainIndex].Position.y < AABB.MinY)
                            {
                                AABB.MinY = TerrainSculptData[TerrainIndex].Position.y;
                            }
                        }
                    }
                }
                bSculptFlushNeeded = true;

                if (bSculptFlushNeeded && bSculpting)
                {
                    const u8 NextVBOIndex = (CurrentSculptingVBOIndex + 1) % NumSculptingVBOs;
                    if (SculptingVBOFences[NextVBOIndex] == nullptr || SculptingVBOFences[NextVBOIndex]->Wait(10))
                    {
                        // Send updated data
                        SculptingVBOs[NextVBOIndex]->Bind(gpu::EBufferBindPoint::WRITE);
                        void* VBOMemoryPtr = SculptingVBOs[NextVBOIndex]->MemoryMap(UNSYNCHRONIZED_WRITE);
                        memcpy(VBOMemoryPtr, TerrainSculptData, TerrainMesh->SubMeshes[0]->VertexDataBuffer.Size);
                        SculptingVBOs[NextVBOIndex]->MemoryUnmap();
                        SculptingVBOs[NextVBOIndex]->Unbind();

                        // Change VBO to the new one
                        TerrainMesh->SubMeshes[0]->VAO->SetVertexBuffer(SculptingVBOs[NextVBOIndex]);

                        bSculptFlushNeeded = false;
                        bSculpFlushed      = true;

                        // delete old fence
                        if (SculptingVBOFences[NextVBOIndex])
                        {
                            SculptingVBOFences[NextVBOIndex]->Free();
                            delete SculptingVBOFences[NextVBOIndex];
                            SculptingVBOFences[NextVBOIndex] = nullptr;
                        }

                        // Update current VBO index
                        CurrentSculptingVBOIndex = NextVBOIndex;
                    }
                }
            }
            else if (ImGui::Button("Sculpt terrain"))
            {
                // Start sculpting when button is pressed
                bSculpting                           = true;
                GSceneEditorState.bBlockActorPicking = true;

                if (!TerrainMesh->IsLoadedToMainMemory())
                {
                    TerrainMesh->LoadDataToMainMemorySynchronously();
                    bShouldFreeMainMemoryAfterSculpting = true;
                }

                // Create VBOs set to stream-draw that we'll cycle through while sculpting
                gpu::FBufferDescription SculptingVBODescription;
                SculptingVBODescription.Offset = 0;
                SculptingVBODescription.Size   = TerrainMesh->SubMeshes[0]->VertexDataBuffer.Size;
                SculptingVBODescription.Data   = TerrainMesh->SubMeshes[0]->VertexDataBuffer.Pointer;

                for (u8 i = 0; i < NumSculptingVBOs; ++i)
                {
                    SculptingVBOs[i] = gpu::CreateBuffer(SculptingVBODescription, gpu::EBufferUsage::STREAM_DRAW, "SculptingVBO");
                }

                OriginalTerrainVBO = TerrainMesh->SubMeshes[0]->VAO->GetVertexBuffer();
                TerrainSculptData  = (FTerrainVertex*)TerrainMesh->SubMeshes[0]->VertexDataBuffer.Pointer;
            }

            if (ImGui::CollapsingHeader("Material"))
            {

                if (TerrainMaterial)
                {
                    ImGuiShowMaterialEditor(TerrainMaterial, &bMaterialEditorOpen);
                    if (!bMaterialEditorOpen)
                    {
                        bMaterialEditorOpen = true;
                    }
                }
            }
        }
    }

    void CTerrain::InternalSaveAssetToFile(const FString& InFilePath)
    {
        FTerrainDescription TerrainDescription;
        FillActorAssetDescription(TerrainDescription);
        WriteToJSONFile(TerrainDescription, *InFilePath);
    }

    CTerrain* CTerrain::CreateAsset(const FDString& InName, const FTerrainSettings& InTerrainSettings)
    {
        gpu::CShader* TerrainShader = GEngine.GetShadersManager().GetShaderByName("Terrain");

        if (!TerrainShader)
        {
            LUCID_LOG(ELogLevel::WARN, "Can't create terrain - no terrain shader")
            return nullptr;
        }

        const UUID        MaterialAssetId   = sole::uuid4();
        FDString          MaterialName      = SPrintf("TerrainMaterial_%s", MaterialAssetId.str().c_str());
        FDString          MaterialAssetPath = SPrintf("assets/materials/%s.asset", *MaterialName);
        CTerrainMaterial* TerrainMaterial   = new CTerrainMaterial{ MaterialAssetId, MaterialName, MaterialAssetPath, TerrainShader };

        GEngine.AddMaterialAsset(TerrainMaterial, EMaterialType::TERRAIN, MaterialAssetPath);

        resources::CMeshResource* TerrainMesh = GenerateTerrainMesh(InTerrainSettings);

        auto* TerrainActorAsset = new CTerrain{ InName, nullptr, nullptr, InTerrainSettings, TerrainMesh, TerrainMaterial, TerrainMesh->GetAABB() };

        TerrainActorAsset->AssetId   = sole::uuid4();
        TerrainActorAsset->AssetPath = SPrintf("assets/actors/%s.asset", *TerrainActorAsset->Name);

        TerrainActorAsset->SaveAssetToFile();

        return TerrainActorAsset;
    }

} // namespace lucid::scene
