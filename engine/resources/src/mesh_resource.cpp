#include "resources/mesh_resource.hpp"

#include "devices/gpu/buffer.hpp"
#include "devices/gpu/texture_enums.hpp"

#include "engine/engine.hpp"

#include "scene/actors/actor.hpp"
#include "scene/material.hpp"
#include "scene/blinn_phong_material.hpp"
#include "scene/actors/actor_enums.hpp"
#include "scene/actors/static_mesh.hpp"

#include "common/log.hpp"
#include "common/bytes.hpp"

#include "glm/glm.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "resources/texture_resource.hpp"
#include "resources/serialization_versions.hpp"

#include "platform/util.hpp"

#include "schemas/types.hpp"

#include <filesystem>

#include "devices/gpu/vao.hpp"

namespace lucid::resources
{
#define SUBMESH_INFO_SIZE (((sizeof(u32) * 4) + (sizeof(bool) * 4)))

    CMeshResource::CMeshResource(const UUID&    InID,
                                 const FString& InName,
                                 const FString& InFilePath,
                                 const u64&     InOffset,
                                 const u64&     InDataSize,
                                 const u32&     InAssetSerializationVersion)
    : CResource(InID, InName, InFilePath, InOffset, InDataSize, InAssetSerializationVersion)
    {
    }

    void CMeshResource::LoadMetadata(FILE* ResourceFile)
    {
        u16 NumSubMeshes;
        fread_s(&NumSubMeshes, sizeof(NumSubMeshes), sizeof(NumSubMeshes), 1, ResourceFile);

        for (u16 i = 0; i < NumSubMeshes; ++i)
        {
            FSubMesh SubMesh;
            fread_s(&SubMesh.VertexDataBuffer.Capacity,
                    sizeof(SubMesh.VertexDataBuffer.Capacity),
                    sizeof(SubMesh.VertexDataBuffer.Capacity),
                    1,
                    ResourceFile);
            fread_s(&SubMesh.ElementDataBuffer.Capacity,
                    sizeof(SubMesh.ElementDataBuffer.Capacity),
                    sizeof(SubMesh.ElementDataBuffer.Capacity),
                    1,
                    ResourceFile);

            fread_s(&SubMesh.VertexCount, sizeof(SubMesh.VertexCount), sizeof(SubMesh.VertexCount), 1, ResourceFile);
            fread_s(&SubMesh.ElementCount, sizeof(SubMesh.ElementCount), sizeof(SubMesh.ElementCount), 1, ResourceFile);

            fread_s(&SubMesh.bHasPositions, sizeof(SubMesh.bHasPositions), sizeof(SubMesh.bHasPositions), 1, ResourceFile);
            fread_s(&SubMesh.bHasNormals, sizeof(SubMesh.bHasNormals), sizeof(SubMesh.bHasNormals), 1, ResourceFile);
            fread_s(&SubMesh.bHasTangetns, sizeof(SubMesh.bHasTangetns), sizeof(SubMesh.bHasTangetns), 1, ResourceFile);
            fread_s(&SubMesh.bHasUVs, sizeof(SubMesh.bHasUVs), sizeof(SubMesh.bHasUVs), 1, ResourceFile);

            if (AssetSerializationVersion > 1)
            {
                fread_s(&SubMesh.MaterialIndex, sizeof(SubMesh.MaterialIndex), sizeof(SubMesh.MaterialIndex), 1, ResourceFile);
            }

            SubMeshes.Add(SubMesh);
        }

        if (AssetSerializationVersion > 0)
        {
            fread_s(&MinPosX, sizeof(MinPosX), sizeof(MinPosX), 1, ResourceFile);
            fread_s(&MaxPosX, sizeof(MaxPosX), sizeof(MaxPosX), 1, ResourceFile);
            fread_s(&MinPosY, sizeof(MinPosY), sizeof(MinPosY), 1, ResourceFile);
            fread_s(&MaxPosY, sizeof(MaxPosY), sizeof(MaxPosY), 1, ResourceFile);
            fread_s(&MinPosZ, sizeof(MinPosZ), sizeof(MinPosZ), 1, ResourceFile);
            fread_s(&MaxPosZ, sizeof(MaxPosZ), sizeof(MaxPosZ), 1, ResourceFile);
        }

        if (AssetSerializationVersion > 2)
        {
            fread_s(&DrawMode, sizeof(DrawMode), sizeof(DrawMode), 1, ResourceFile);            
        }
    }

    void CMeshResource::LoadDataToMainMemorySynchronously()
    {
        if (bLoadedToMainMemory)
        {
            return;
        }

        FILE* MeshFile;
        if (fopen_s(&MeshFile, *FilePath, "rb") != 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to open file %s", *FilePath);
            return;
        }
        // Position the file pointer to the beginning of mesh data
        u32 VertexDataOffset = Offset + RESOURCE_FILE_HEADER_SIZE + Name.GetLength() + sizeof(u16)  + (SUBMESH_INFO_SIZE * SubMeshes.GetLength());

        if (AssetSerializationVersion > 0)
        {
            VertexDataOffset += sizeof(float) * 6;
        }

        if (AssetSerializationVersion > 1)
        {
            VertexDataOffset += sizeof(u8) * SubMeshes.GetLength();
        }

        if (AssetSerializationVersion > 2)
        {
            VertexDataOffset += sizeof(DrawMode);
        }

        fseek(MeshFile, VertexDataOffset, SEEK_SET);

        // Read vertex and element data for each submesh
        for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
        {
            FSubMesh* SubMesh = SubMeshes[i];

            SubMesh->VertexDataBuffer.Pointer = (char*)malloc(SubMesh->VertexDataBuffer.Capacity);
            u64 NumElementsRead =
              fread_s(SubMesh->VertexDataBuffer.Pointer, SubMesh->VertexDataBuffer.Capacity, SubMesh->VertexDataBuffer.Capacity, 1, MeshFile);
            assert(NumElementsRead == 1);
            SubMesh->VertexDataBuffer.Size = SubMesh->VertexDataBuffer.Capacity;

            // Read element data
            if (SubMesh->ElementDataBuffer.Capacity > 0)
            {
                SubMesh->ElementDataBuffer.Pointer = (char*)malloc(SubMesh->ElementDataBuffer.Capacity);
                NumElementsRead =
                  fread_s(SubMesh->ElementDataBuffer.Pointer, SubMesh->ElementDataBuffer.Capacity, SubMesh->ElementDataBuffer.Capacity, 1, MeshFile);
                assert(NumElementsRead == 1);
                SubMesh->ElementDataBuffer.Size = SubMesh->ElementDataBuffer.Capacity;
            }
        }

        // Close the file
        fclose(MeshFile);
        bLoadedToMainMemory = true;
        IsVideoMemoryFreed  = false;
    }

    void CMeshResource::LoadDataToVideoMemorySynchronously()
    {
        if (bLoadedToVideoMemory)
        {
            return;
        }

        // For now we require for the data to be loaded in the main memory
        if (!bLoadedToMainMemory)
        {
            LoadDataToMainMemorySynchronously();
        }

        for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
        {
            FSubMesh* SubMesh = SubMeshes[i];

            gpu::FBufferDescription GPUBufferDescription;

            // Sending vertex data to the gpu
            GPUBufferDescription.Data = SubMesh->VertexDataBuffer.Pointer;
            GPUBufferDescription.Size = SubMesh->VertexDataBuffer.Size;

            SubMesh->VertexBuffer = gpu::CreateBuffer(GPUBufferDescription, gpu::EBufferUsage::STATIC_DRAW, SPrintf("%s_VertexBuffer_%d", *Name, i));
            assert(SubMesh->VertexBuffer);
            // Sending element to the gpu if it's present
            if (SubMesh->ElementDataBuffer.Pointer)
            {
                GPUBufferDescription.Data = SubMesh->ElementDataBuffer.Pointer;
                GPUBufferDescription.Size = SubMesh->ElementDataBuffer.Size;

                SubMesh->ElementBuffer =
                  gpu::CreateBuffer(GPUBufferDescription, gpu::EBufferUsage::STATIC_DRAW, SPrintf("%s_ElementBuffer_%d", *Name, i));
                assert(SubMesh->ElementBuffer);
            }

            FArray<gpu::FVertexAttribute> MeshAttributes(4);

            u16 Stride = 0;
            if (SubMesh->bHasPositions)
            {
                Stride += sizeof(float) * 3;
            }

            if (SubMesh->bHasNormals)
            {
                Stride += sizeof(float) * 3;
            }

            if (SubMesh->bHasTangetns)
            {
                Stride += sizeof(float) * 3;
            }

            if (SubMesh->bHasUVs)
            {
                Stride += sizeof(float) * 2;
            }

            u8 FirstElemOffset = 0;
            if (SubMesh->bHasPositions)
            {
                MeshAttributes.Add({ 0, 3, EType::FLOAT, false, Stride, FirstElemOffset, 0 });
                FirstElemOffset += sizeof(float) * 3;
            }

            if (SubMesh->bHasNormals)
            {
                MeshAttributes.Add({ 1, 3, EType::FLOAT, false, Stride, FirstElemOffset, 0 });
                FirstElemOffset += sizeof(float) * 3;
            }

            if (SubMesh->bHasTangetns)
            {
                MeshAttributes.Add({ 2, 3, EType::FLOAT, false, Stride, FirstElemOffset, 0 });
                FirstElemOffset += sizeof(float) * 3;
            }

            if (SubMesh->bHasUVs)
            {
                MeshAttributes.Add({ 3, 2, EType::FLOAT, false, Stride, FirstElemOffset, 0 });
            }

            SubMesh->VAO = gpu::CreateVertexArray(SPrintf("%s_VAO_%d", *Name, i),
                                                  &MeshAttributes,
                                                  SubMesh->VertexBuffer,
                                                  SubMesh->ElementBuffer,
                                                  DrawMode,
                                                  SubMesh->VertexCount,
                                                  SubMesh->ElementCount);
            assert(SubMesh->VAO);

            MeshAttributes.Free();
        }

        bLoadedToVideoMemory = true;
        IsMainMemoryFreed    = false;
    }

    void CMeshResource::SaveSynchronously(FILE* ResourceFile) const
    {
        assert(SubMeshes.GetLength());

        // Write header
        SaveHeader(ResourceFile);

        // Save metadata about of vertex and element buffers
        const u16 NumSubMeshes = SubMeshes.GetLength();
        fwrite(&NumSubMeshes, sizeof(NumSubMeshes), 1, ResourceFile);

        for (u16 i = 0; i < NumSubMeshes; ++i)
        {
            fwrite(&SubMeshes[i]->VertexDataBuffer.Capacity, sizeof(SubMeshes[i]->VertexDataBuffer.Capacity), 1, ResourceFile);
            fwrite(&SubMeshes[i]->ElementDataBuffer.Capacity, sizeof(SubMeshes[i]->ElementDataBuffer.Capacity), 1, ResourceFile);
            fwrite(&SubMeshes[i]->VertexCount, sizeof(SubMeshes[i]->VertexCount), 1, ResourceFile);
            fwrite(&SubMeshes[i]->ElementCount, sizeof(SubMeshes[i]->ElementCount), 1, ResourceFile);

            fwrite(&SubMeshes[i]->bHasPositions, sizeof(SubMeshes[i]->bHasPositions), 1, ResourceFile);
            fwrite(&SubMeshes[i]->bHasNormals, sizeof(SubMeshes[i]->bHasNormals), 1, ResourceFile);
            fwrite(&SubMeshes[i]->bHasTangetns, sizeof(SubMeshes[i]->bHasTangetns), 1, ResourceFile);
            fwrite(&SubMeshes[i]->bHasUVs, sizeof(SubMeshes[i]->bHasUVs), 1, ResourceFile);
            fwrite(&SubMeshes[i]->MaterialIndex, sizeof(SubMeshes[i]->MaterialIndex), 1, ResourceFile);
        }

        fwrite(&MinPosX, sizeof(MinPosX), 1, ResourceFile);
        fwrite(&MaxPosX, sizeof(MaxPosX), 1, ResourceFile);
        fwrite(&MinPosY, sizeof(MinPosY), 1, ResourceFile);
        fwrite(&MaxPosY, sizeof(MaxPosY), 1, ResourceFile);
        fwrite(&MinPosZ, sizeof(MinPosZ), 1, ResourceFile);
        fwrite(&MaxPosZ, sizeof(MaxPosZ), 1, ResourceFile);

        fwrite(&DrawMode, sizeof(gpu::EDrawMode), 1, ResourceFile);
        
        // Save vertex and data for each submesh
        for (u16 i = 0; i < NumSubMeshes; ++i)
        {
            fwrite(SubMeshes[i]->VertexDataBuffer.Pointer, SubMeshes[i]->VertexDataBuffer.Size, 1, ResourceFile);
            if (SubMeshes[i]->ElementDataBuffer.Pointer)
            {
                fwrite(SubMeshes[i]->ElementDataBuffer.Pointer, SubMeshes[i]->ElementDataBuffer.Size, 1, ResourceFile);
            }
        }

    }

    void CMeshResource::FreeMainMemory()
    {
        if (bLoadedToMainMemory && !IsMainMemoryFreed)
        {
            for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
            {
                free(SubMeshes[i]->VertexDataBuffer.Pointer);
                free(SubMeshes[i]->ElementDataBuffer.Pointer);
            }

            IsMainMemoryFreed   = true;
            bLoadedToMainMemory = false;
        }
    }

    void CMeshResource::FreeVideoMemory()
    {
        if (bLoadedToVideoMemory && !IsVideoMemoryFreed)
        {
            for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
            {
                SubMeshes[i]->VAO->Free();

                delete SubMeshes[i]->VAO;
                delete SubMeshes[i]->VertexBuffer;
                delete SubMeshes[i]->ElementBuffer;
            }

            IsVideoMemoryFreed   = true;
            bLoadedToVideoMemory = false;
        }
    }

    CResource* CMeshResource::CreateCopy() const
    {
        assert(0); //@TODO
        return nullptr;
    }

    CResourcesHolder<CMeshResource> MeshesHolder;

    static Assimp::Importer AssimpImporter;

    static constexpr u32 ASSIMP_DEFAULT_FLAGS =
      aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices;

    /* Helper structure used when importing the mesh */
    struct FMeshImportInfo
    {
        aiString Name;

        bool bHasPositions = false;
        bool bHasNormals   = false;
        bool bHasTangents  = false;
        bool bHasUVs       = false;

        FMemBuffer VertexData;
        FMemBuffer ElementData;

        u32 VertexCount  = 0;
        u32 ElementCount = 0;

        u8 MaterialIndex = 0;

        u16 VertexSize = 0;
    };

    /* Helper structure used when importing the mesh */
    struct FMeshInfoHelper
    {
        FArray<FMeshImportInfo> SubMeshes{ 1, true };
        /**
         * Mapping used to combine meshes using the same material to a single VBO
         * so we can reduce the number of draw calls
         */
        float MinX = 0, MaxX = 0;
        float MinY = 0, MaxY = 0;
        float MinZ = 0, MaxZ = 0;
    };

    /* Helper structure used when importing the mesh */
    struct FMeshDataSize
    {
        u32 VertexDataSize  = 0;
        u32 ElementDataSize = 0;
    };

    static FMeshDataSize AssimpCalculateMeshDataSize(aiNode* Node, const aiScene* Scene)
    {
        FMeshDataSize MeshSize;
        for (u32 idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            aiMesh* MeshNode   = Scene->mMeshes[Node->mMeshes[idx]];
            u32     vertexSize = 0;

            assert(MeshNode->HasPositions());
            assert(MeshNode->HasNormals());
            assert(MeshNode->HasNormals());
            assert(MeshNode->HasTextureCoords(0));

            // Position
            vertexSize += sizeof(glm::vec3);
            // Normals
            vertexSize += sizeof(glm::vec3);
            // Tangents
            vertexSize += sizeof(glm::vec3);
            // UV
            vertexSize += sizeof(glm::vec2);

            MeshSize.VertexDataSize += MeshNode->mNumVertices * vertexSize;
            MeshSize.ElementDataSize += (MeshNode->mNumFaces * 3 * sizeof(u32));
        }

        // Recursively calculate size of the children
        for (unsigned int idx = 0; idx < Node->mNumChildren; ++idx)
        {
            const FMeshDataSize SubMeshSize = AssimpCalculateMeshDataSize(Node->mChildren[idx], Scene);
            MeshSize.VertexDataSize += SubMeshSize.VertexDataSize;
            MeshSize.ElementDataSize += SubMeshSize.ElementDataSize;
        }

        return MeshSize;
    }

    static void AddAssetsToEngine(scene::CStaticMesh* InStaticMesh, CMeshResource* InMesh)
    {
        // Save actor to file
        InStaticMesh->SaveToResourceFile();

        // Save mesh to file
        FILE* ImportedMeshFile;
        fopen_s(&ImportedMeshFile, *InMesh->GetFilePath(), "wb");
        InMesh->SaveSynchronously(ImportedMeshFile);
        fclose(ImportedMeshFile);

        // Add assets and resoruce to the engine
        GEngine.AddActorAsset(InStaticMesh);
        GEngine.AddMeshResource(InMesh);
    }

    static void LoadAssimpNode(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData);
    static void LoadAssimpNodeAsSingleMesh(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData, FMeshImportInfo& CombinedMeshInfo);
    static void LoadAssimpMesh(aiMesh* Mesh, FMeshInfoHelper& MeshData);
    static void LoadAssimpMeshAsSingleMesh(aiMesh* Mesh, FMeshInfoHelper& MeshData, FMeshImportInfo& CombinedMesh);

    static CTextureResource* AssimpImportMaterialTexture(const u8&      InIndex,
                                                         const FString& ModelFileDir,
                                                         aiMaterial*    Material,
                                                         aiTextureType  TextureType,
                                                         const FString& MeshName,
                                                         const FString& TextureTypeName,
                                                         const bool&    InFlipUV);

    FArray<CMeshResource*>
    ImportMesh(const FString& InMeshFilePath, const FString& MeshName, const bool& InbFilpUVs, const EMeshImportStretegy& InMeshImportStrategy)
    {
#ifndef NDEBUG
        real StartTime = platform::GetCurrentTimeSeconds();
#endif

        auto           AssimpFlags = ASSIMP_DEFAULT_FLAGS;
        const aiScene* Root        = AssimpImporter.ReadFile(*InMeshFilePath, AssimpFlags);

        LUCID_LOG(ELogLevel::INFO, "Reading mesh with assimp %s took %f", *InMeshFilePath, platform::GetCurrentTimeSeconds() - StartTime);

        if (!Root || Root->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Root->mRootNode)
        {
            LUCID_LOG(ELogLevel::WARN, "Assimp failed to load model %s", AssimpImporter.GetErrorString())
            return { 0, false };
        }

        FMeshInfoHelper MeshInfoHelper;

#ifndef NDEBUG
        StartTime = platform::GetCurrentTimeSeconds();
#endif
        MeshInfoHelper.MinX = MeshInfoHelper.MinY = MeshInfoHelper.MinZ = FLT_MAX;
        MeshInfoHelper.MaxX = MeshInfoHelper.MaxY = MeshInfoHelper.MaxZ = 0;

        const FMeshDataSize MeshDataSize = AssimpCalculateMeshDataSize(Root->mRootNode, Root);

        std::filesystem::path MeshFilePath{ *InMeshFilePath };
        FDString              MeshFileDirPath = CopyToString(MeshFilePath.parent_path().string().c_str()); // @TODO .string().c_str()
                                                                                              // once we support wchars

        FArray<CMeshResource*> ImportedMeshes{ InMeshImportStrategy == EMeshImportStretegy::SPLIT_MESHES ? Root->mNumMeshes : 1, false };

        // Load meshes to main memory and create MeshResources for them
        if (InMeshImportStrategy == EMeshImportStretegy::SINGLE_MESH)
        {
            auto* ImportedMesh = new CMeshResource{ sole::uuid4(),
                                                    MeshName,
                                                    SPrintf("assets/meshes/%s.asset", *MeshName),
                                                    0,
                                                    MeshDataSize.VertexDataSize + MeshDataSize.ElementDataSize,
                                                    MESH_SERIALIZATION_VERSION };
            ImportedMeshes.Add(ImportedMesh);

            FMeshImportInfo CombinedMeshInfo{};
            CombinedMeshInfo.VertexData = CreateMemBuffer(MeshDataSize.VertexDataSize);
            if (MeshDataSize.ElementDataSize > 0)
            {
                CombinedMeshInfo.ElementData = CreateMemBuffer(MeshDataSize.ElementDataSize);
            }

            LoadAssimpNodeAsSingleMesh(Root->mRootNode, Root, MeshInfoHelper, CombinedMeshInfo);

            FSubMesh CombinedMesh;
            CombinedMesh.bHasPositions     = Root->mMeshes[0]->HasPositions();
            CombinedMesh.bHasNormals       = Root->mMeshes[0]->HasNormals();
            CombinedMesh.bHasTangetns      = Root->mMeshes[0]->HasTangentsAndBitangents();
            CombinedMesh.bHasUVs           = Root->mMeshes[0]->HasTextureCoords(0);
            CombinedMesh.VertexDataBuffer  = CombinedMeshInfo.VertexData;
            CombinedMesh.ElementDataBuffer = CombinedMeshInfo.ElementData;
            CombinedMesh.VertexCount       = CombinedMeshInfo.VertexCount;
            CombinedMesh.ElementCount      = CombinedMeshInfo.ElementCount;
            CombinedMesh.MaterialIndex     = 0;
            ImportedMesh->SubMeshes.Add(CombinedMesh);

            ImportedMesh->MinPosX = MeshInfoHelper.MinX;
            ImportedMesh->MaxPosX = MeshInfoHelper.MaxX;

            ImportedMesh->MinPosY = MeshInfoHelper.MinY;
            ImportedMesh->MaxPosY = MeshInfoHelper.MaxY;

            ImportedMesh->MinPosZ = MeshInfoHelper.MinZ;
            ImportedMesh->MaxPosZ = MeshInfoHelper.MaxZ;
        }
        else
        {
            LoadAssimpNode(Root->mRootNode, Root, MeshInfoHelper);

            if (InMeshImportStrategy == EMeshImportStretegy::SPLIT_MESHES)
            {
                for (u16 i = 0; i < MeshInfoHelper.SubMeshes.GetLength(); ++i)
                {
                    const auto& SubMeshInfo  = MeshInfoHelper.SubMeshes[i];
                    auto        SubMeshName  = SPrintf("%s_%s", *MeshName, SubMeshInfo->Name.C_Str());
                    auto*       ImportedMesh = new CMeshResource{ sole::uuid4(),
                                                            SubMeshName,
                                                            SPrintf("assets/meshes/%s.asset", *SubMeshName, SubMeshInfo->Name.C_Str()),
                                                            0,
                                                            SubMeshInfo->VertexData.Size + SubMeshInfo->ElementData.Size,
                                                            MESH_SERIALIZATION_VERSION };
                    ImportedMeshes.Add(ImportedMesh);

                    FSubMesh SubMesh;
                    SubMesh.MaterialIndex     = 0;
                    SubMesh.bHasPositions     = SubMeshInfo->bHasPositions;
                    SubMesh.bHasNormals       = SubMeshInfo->bHasNormals;
                    SubMesh.bHasTangetns      = SubMeshInfo->bHasTangents;
                    SubMesh.bHasUVs           = SubMeshInfo->bHasUVs;
                    SubMesh.VertexDataBuffer  = SubMeshInfo->VertexData;
                    SubMesh.ElementDataBuffer = SubMeshInfo->ElementData;
                    SubMesh.VertexCount       = SubMeshInfo->VertexCount;
                    SubMesh.ElementCount      = SubMeshInfo->ElementCount;
                    ImportedMesh->SubMeshes.Add(SubMesh);

                    ImportedMesh->MinPosX = MeshInfoHelper.MinX;
                    ImportedMesh->MaxPosX = MeshInfoHelper.MaxX;

                    ImportedMesh->MinPosY = MeshInfoHelper.MinY;
                    ImportedMesh->MaxPosY = MeshInfoHelper.MaxY;

                    ImportedMesh->MinPosZ = MeshInfoHelper.MinZ;
                    ImportedMesh->MaxPosZ = MeshInfoHelper.MaxZ;
                }
            }
            else
            {
                auto* ImportedMesh = new CMeshResource{ sole::uuid4(),
                                                        MeshName,
                                                        SPrintf("assets/meshes/%s.asset", *MeshName),
                                                        0,
                                                        MeshDataSize.VertexDataSize + MeshDataSize.ElementDataSize,
                                                        MESH_SERIALIZATION_VERSION };
                ImportedMeshes.Add(ImportedMesh);

                for (u16 i = 0; i < MeshInfoHelper.SubMeshes.GetLength(); ++i)
                {
                    FSubMesh SubMesh;
                    SubMesh.MaterialIndex     = MeshInfoHelper.SubMeshes[i]->MaterialIndex;
                    SubMesh.bHasPositions     = MeshInfoHelper.SubMeshes[i]->bHasPositions;
                    SubMesh.bHasNormals       = MeshInfoHelper.SubMeshes[i]->bHasNormals;
                    SubMesh.bHasTangetns      = MeshInfoHelper.SubMeshes[i]->bHasTangents;
                    SubMesh.bHasUVs           = MeshInfoHelper.SubMeshes[i]->bHasUVs;
                    SubMesh.VertexDataBuffer  = MeshInfoHelper.SubMeshes[i]->VertexData;
                    SubMesh.ElementDataBuffer = MeshInfoHelper.SubMeshes[i]->ElementData;
                    SubMesh.VertexCount       = MeshInfoHelper.SubMeshes[i]->VertexCount;
                    SubMesh.ElementCount      = MeshInfoHelper.SubMeshes[i]->ElementCount;
                    ImportedMesh->SubMeshes.Add(SubMesh);
                }

                ImportedMesh->MinPosX = MeshInfoHelper.MinX;
                ImportedMesh->MaxPosX = MeshInfoHelper.MaxX;

                ImportedMesh->MinPosY = MeshInfoHelper.MinY;
                ImportedMesh->MaxPosY = MeshInfoHelper.MaxY;

                ImportedMesh->MinPosZ = MeshInfoHelper.MinZ;
                ImportedMesh->MaxPosZ = MeshInfoHelper.MaxZ;
            }
        }

        // Create actors for the meshes
        if (InMeshImportStrategy == EMeshImportStretegy::SPLIT_MESHES)
        {
            FArray<scene::CMaterial*> ImportedMaterials{ 1, true };

            // Load materials
            for (int MaterialIndex = 0; MaterialIndex < Root->mNumMaterials; ++MaterialIndex)
            {
                aiMaterial* Material = Root->mMaterials[MaterialIndex];

                aiString MaterialName;
                Material->Get(AI_MATKEY_NAME, MaterialName);
                if (MaterialName == aiString("Default"))
                {
                    continue;
                }

#ifndef NDEBUG
                StartTime = platform::GetCurrentTimeSeconds();
#endif
                CTextureResource* DiffuseMap =
                  AssimpImportMaterialTexture(0, MeshFileDirPath, Material, aiTextureType_DIFFUSE, MeshName, FString{ "Diffuse" }, InbFilpUVs);
                CTextureResource* SpecularMap =
                  AssimpImportMaterialTexture(0, MeshFileDirPath, Material, aiTextureType_SPECULAR, MeshName, FString{ "Specular" }, InbFilpUVs);
                CTextureResource* NormalMap =
                  AssimpImportMaterialTexture(0, MeshFileDirPath, Material, aiTextureType_HEIGHT, MeshName, FString{ "Normal" }, InbFilpUVs);

                auto* ImportedMaterial = new scene::CBlinnPhongMapsMaterial{ sole::uuid4(),
                                                                             SPrintf("%s_%s", *MeshName, MaterialName.C_Str()),
                                                                             SPrintf("assets/materials/%s_%s.asset", *MeshName, MaterialName.C_Str()),
                                                                             GEngine.GetShadersManager().GetShaderByName("BlinnPhongMaps") };

                ImportedMaterial->SetShininess(32);
                ImportedMaterial->SetDiffuseMap(DiffuseMap);
                ImportedMaterial->SetSpecularMap(SpecularMap);
                ImportedMaterial->SetNormalMap(NormalMap);
                ImportedMaterial->SetDisplacementMap(nullptr);
                ImportedMaterial->bIsAsset = true;

                ImportedMaterials.Add(ImportedMaterial);

                // Add material do the engine
                ImportedMaterial->SaveToResourceFile(EFileFormat::Json);
                GEngine.AddMaterialAsset(ImportedMaterial, scene::EMaterialType::BLINN_PHONG_MAPS, ImportedMaterial->ResourcePath);
            }

            // Create a static mesh actor for each of the submeshes
            for (u32 i = 0; i < ImportedMeshes.GetLength(); ++i)
            {
                auto* ImportedMesh         = *ImportedMeshes[i];
                auto* StaticMeshActorAsset = new scene::CStaticMesh{ SPrintf("%s_%s", *MeshName, MeshInfoHelper.SubMeshes[i]->Name.C_Str()),
                                                                     nullptr,
                                                                     nullptr,
                                                                     ImportedMesh,
                                                                     scene::EStaticMeshType::STATIONARY };

                StaticMeshActorAsset->ResourceId   = sole::uuid4();
                StaticMeshActorAsset->ResourcePath = SPrintf("assets/actors/%s.asset", *StaticMeshActorAsset->Name);

                StaticMeshActorAsset->AddMaterial(*ImportedMaterials[MeshInfoHelper.SubMeshes[i]->MaterialIndex]);
                AddAssetsToEngine(StaticMeshActorAsset, ImportedMesh);
            }

            ImportedMaterials.Free();
        }
        else
        {
            // Create a static mesh actor for this mesh
            auto* StaticMeshActorAsset = new scene::CStaticMesh{
                CopyToString(*MeshName, MeshName.GetLength()), nullptr, nullptr, *ImportedMeshes[0], scene::EStaticMeshType::STATIONARY
            };
            StaticMeshActorAsset->ResourceId   = sole::uuid4();
            StaticMeshActorAsset->ResourcePath = SPrintf("assets/actors/%s.asset", *MeshName);

            // Load materials
            for (int MaterialIndex = 0; MaterialIndex < Root->mNumMaterials; ++MaterialIndex)
            {
                aiMaterial* Material = Root->mMaterials[MaterialIndex];

                aiString MaterialName;
                Material->Get(AI_MATKEY_NAME, MaterialName);

#ifndef NDEBUG
                StartTime = platform::GetCurrentTimeSeconds();
#endif
                CTextureResource* DiffuseMap =
                  AssimpImportMaterialTexture(0, MeshFileDirPath, Material, aiTextureType_DIFFUSE, MeshName, FString{ "Diffuse" }, InbFilpUVs);
                CTextureResource* SpecularMap =
                  AssimpImportMaterialTexture(0, MeshFileDirPath, Material, aiTextureType_SPECULAR, MeshName, FString{ "Specular" }, InbFilpUVs);
                CTextureResource* NormalMap =
                  AssimpImportMaterialTexture(0, MeshFileDirPath, Material, aiTextureType_HEIGHT, MeshName, FString{ "Normal" }, InbFilpUVs);

                auto* ImportedMaterial = new scene::CBlinnPhongMapsMaterial{ sole::uuid4(),
                                                                             SPrintf("%s_%s", *MeshName, MaterialName.C_Str()),
                                                                             SPrintf("assets/materials/%s_%s.asset", *MeshName, MaterialName.C_Str()),
                                                                             GEngine.GetShadersManager().GetShaderByName("BlinnPhongMaps") };
                ImportedMaterial->SetShininess(32);
                ImportedMaterial->SetDiffuseMap(DiffuseMap);
                ImportedMaterial->SetSpecularMap(SpecularMap);
                ImportedMaterial->SetNormalMap(NormalMap);
                ImportedMaterial->SetDisplacementMap(nullptr);
                ImportedMaterial->bIsAsset = true;

                // Use the first material (this should also be the only one in the mesh btw) as the material for the imported mesh
                if (InMeshImportStrategy == EMeshImportStretegy::SINGLE_MESH && MaterialIndex == 0)
                {
                    StaticMeshActorAsset->AddMaterial(ImportedMaterial);
                }
                else
                {
                    StaticMeshActorAsset->AddMaterial(ImportedMaterial);
                }

                // Add material do the engine
                ImportedMaterial->SaveToResourceFile(EFileFormat::Json);
                GEngine.AddMaterialAsset(ImportedMaterial, scene::EMaterialType::BLINN_PHONG_MAPS, ImportedMaterial->ResourcePath);
            }

            AddAssetsToEngine(StaticMeshActorAsset, StaticMeshActorAsset->MeshResource);
        }

        MeshFileDirPath.Free();

#ifndef NDEBUG
        LUCID_LOG(ELogLevel::INFO, "Loading textures of mesh %s took %f", *MeshName, platform::GetCurrentTimeSeconds() - StartTime);
        LUCID_LOG(ELogLevel::INFO, "Sending mesh %s data to GPU took %f", *MeshName, platform::GetCurrentTimeSeconds() - StartTime);
#endif

        return ImportedMeshes;
    }; // namespace lucid::resources

    static void LoadAssimpNode(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData)
    {
        for (u32 idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            LoadAssimpMesh(mesh, MeshData);
        }

        for (u32 idx = 0; idx < Node->mNumChildren; ++idx)
        {
            LoadAssimpNode(Node->mChildren[idx], Scene, MeshData);
        }
    }

    static void LoadAssimpNodeAsSingleMesh(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData, FMeshImportInfo& CombinedMeshInfo)
    {
        for (u32 idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            LoadAssimpMeshAsSingleMesh(mesh, MeshData, CombinedMeshInfo);
        }

        for (u32 idx = 0; idx < Node->mNumChildren; ++idx)
        {
            LoadAssimpNodeAsSingleMesh(Node->mChildren[idx], Scene, MeshData, CombinedMeshInfo);
        }
    }

    void LoadAssimpMesh(aiMesh* Mesh, FMeshInfoHelper& MeshData)
    {
        u32              ElementDataOffset = 0;
        FMeshImportInfo* SubMeshInfo       = nullptr;

        // Check if we already have as submesh using this material
        for (u32 i = 0; i < MeshData.SubMeshes.GetLength(); ++i)
        {
            if (MeshData.SubMeshes[i]->MaterialIndex == Mesh->mMaterialIndex)
            {
                SubMeshInfo = MeshData.SubMeshes[i];
                break;
            }
        }

        // If we have - combine meshes data
        if (SubMeshInfo)
        {
            if (SubMeshInfo->ElementCount)
            {
                assert(Mesh->mNumFaces);
            }

            // Create a bigger vertex and element buffers to accomodate for the new data
            const u32 VertexDataSize  = (SubMeshInfo->VertexCount + Mesh->mNumVertices) * SubMeshInfo->VertexSize;
            const u32 ElementDataSize = (SubMeshInfo->ElementCount + (Mesh->mNumFaces * 3)) * sizeof(u32);

            FMemBuffer NewVertexBuffer  = CreateMemBuffer(VertexDataSize);
            FMemBuffer NewElementBuffer = ElementDataSize > 0 ? CreateMemBuffer(ElementDataSize) : FMemBuffer{};

            // Copy current vertex data
            memcpy(NewVertexBuffer.Pointer, SubMeshInfo->VertexData.Pointer, SubMeshInfo->VertexData.Size);
            NewVertexBuffer.Size = SubMeshInfo->VertexData.Size;
            SubMeshInfo->VertexData.Free();
            SubMeshInfo->VertexData = NewVertexBuffer;

            if (SubMeshInfo->ElementCount)
            {
                // Copy current element data
                memcpy(NewElementBuffer.Pointer, SubMeshInfo->ElementData.Pointer, SubMeshInfo->ElementData.Size);
                NewElementBuffer.Size = SubMeshInfo->ElementData.Size;
                SubMeshInfo->ElementData.Free();
                SubMeshInfo->ElementData = NewElementBuffer;
            }

            ElementDataOffset = SubMeshInfo->VertexCount;

            SubMeshInfo->VertexCount += Mesh->mNumVertices;
            SubMeshInfo->ElementCount += (Mesh->mNumFaces * 3);
        }
        else
        {
            FMeshImportInfo NewSubMesh;
            NewSubMesh.MaterialIndex = Mesh->mMaterialIndex;
            NewSubMesh.Name          = Mesh->mName;

            if (Mesh->HasPositions())
            {
                NewSubMesh.VertexSize += sizeof(glm::vec3);
                NewSubMesh.bHasPositions = true;
            }

            if (Mesh->HasNormals())
            {
                NewSubMesh.VertexSize += sizeof(glm::vec3);
                NewSubMesh.bHasNormals = true;
            }

            if (Mesh->HasTangentsAndBitangents())
            {
                NewSubMesh.VertexSize += sizeof(glm::vec3);
                NewSubMesh.bHasTangents = true;
            }

            if (Mesh->HasTextureCoords(0))
            {
                NewSubMesh.VertexSize += sizeof(glm::vec2);
                NewSubMesh.bHasUVs = true;
            }

            const u32 VertexDataSize  = Mesh->mNumVertices * NewSubMesh.VertexSize;
            const u32 ElementDataSize = (Mesh->mNumFaces * 3 * sizeof(u32));

            NewSubMesh.VertexCount  = Mesh->mNumVertices;
            NewSubMesh.ElementCount = Mesh->mNumFaces * 3;

            NewSubMesh.VertexData  = CreateMemBuffer(VertexDataSize);
            NewSubMesh.ElementData = ElementDataSize > 0 ? CreateMemBuffer(ElementDataSize) : FMemBuffer{};

            MeshData.SubMeshes.Add(NewSubMesh);
            SubMeshInfo = MeshData.SubMeshes.Last();
        }

        for (uint32_t i = 0; i < Mesh->mNumVertices; ++i)
        {
            glm::vec3* VertexDataPointer = (glm::vec3*)(SubMeshInfo->VertexData.Pointer + SubMeshInfo->VertexData.Size);

            // @TODO Extract the bHasXXX out of the loop to avoid so many checks

            // Position
            if (SubMeshInfo->bHasPositions)
            {
                VertexDataPointer->x = Mesh->mVertices[i].x;
                VertexDataPointer->y = Mesh->mVertices[i].y;
                VertexDataPointer->z = Mesh->mVertices[i].z;

                MeshData.MinX = VertexDataPointer->x < MeshData.MinX ? VertexDataPointer->x : MeshData.MinX;
                MeshData.MaxX = VertexDataPointer->x > MeshData.MaxX ? VertexDataPointer->x : MeshData.MaxX;

                MeshData.MinY = VertexDataPointer->y < MeshData.MinY ? VertexDataPointer->y : MeshData.MinY;
                MeshData.MaxY = VertexDataPointer->y > MeshData.MaxY ? VertexDataPointer->y : MeshData.MaxY;

                MeshData.MinZ = VertexDataPointer->z < MeshData.MinZ ? VertexDataPointer->z : MeshData.MinZ;
                MeshData.MaxZ = VertexDataPointer->z > MeshData.MaxZ ? VertexDataPointer->z : MeshData.MaxZ;

                VertexDataPointer += 1;
                SubMeshInfo->VertexData.Size += sizeof(glm::vec3);
            }

            if (SubMeshInfo->bHasNormals)
            {
                VertexDataPointer->x = Mesh->mNormals[i].x;
                VertexDataPointer->y = Mesh->mNormals[i].y;
                VertexDataPointer->z = Mesh->mNormals[i].z;
                VertexDataPointer += 1;
                SubMeshInfo->VertexData.Size += sizeof(glm::vec3);
            }

            if (SubMeshInfo->bHasTangents)
            {
                VertexDataPointer->x = Mesh->mTangents[i].x;
                VertexDataPointer->y = Mesh->mTangents[i].y;
                VertexDataPointer->z = Mesh->mTangents[i].z;

                VertexDataPointer += 1;
                SubMeshInfo->VertexData.Size += sizeof(glm::vec3);
            }

            if (SubMeshInfo->bHasUVs)
            {
                glm::vec2* texPtr = (glm::vec2*)VertexDataPointer;

                texPtr->x         = Mesh->mTextureCoords[0][i].x;
                texPtr->y         = Mesh->mTextureCoords[0][i].y;
                VertexDataPointer = (glm::vec3*)(texPtr + 1);
                SubMeshInfo->VertexData.Size += sizeof(glm::vec2);
            }
        }

        // Now wak through each of the mesh's faces (a face
        // in a mesh it's triangle) and retrieve the
        // corresponding vertex indices.
        for (unsigned int idx = 0; idx < Mesh->mNumFaces; ++idx)
        {
            aiFace* Face = Mesh->mFaces + idx;

            // Copy the face's indices to the element buffer
            u32*           ElementPtr   = (uint32_t*)(SubMeshInfo->ElementData.Pointer + SubMeshInfo->ElementData.Size);
            const uint32_t FaceDataSize = 3 * sizeof(uint32_t);

            ElementPtr[0] = ElementDataOffset + Face->mIndices[0];
            ElementPtr[1] = ElementDataOffset + Face->mIndices[1];
            ElementPtr[2] = ElementDataOffset + Face->mIndices[2];

            SubMeshInfo->ElementData.Size += FaceDataSize;
        }
    }

    void LoadAssimpMeshAsSingleMesh(aiMesh* Mesh, FMeshInfoHelper& MeshData, FMeshImportInfo& CombinedMesh)
    {
        uint32_t CurrentTotalElementCount = CombinedMesh.VertexCount;
        CombinedMesh.VertexCount += Mesh->mNumVertices;
        CombinedMesh.ElementCount += Mesh->mNumFaces * 3;

        for (uint32_t i = 0; i < Mesh->mNumVertices; ++i)
        {
            glm::vec3* VertexDataPointer = (glm::vec3*)(CombinedMesh.VertexData.Pointer + CombinedMesh.VertexData.Size);

            VertexDataPointer->x = Mesh->mVertices[i].x;
            VertexDataPointer->y = Mesh->mVertices[i].y;
            VertexDataPointer->z = Mesh->mVertices[i].z;

            VertexDataPointer += 1;
            CombinedMesh.VertexData.Size += sizeof(glm::vec3);

            MeshData.MinX = VertexDataPointer->x < MeshData.MinX ? VertexDataPointer->x : MeshData.MinX;
            MeshData.MaxX = VertexDataPointer->x > MeshData.MaxX ? VertexDataPointer->x : MeshData.MaxX;

            MeshData.MinY = VertexDataPointer->y < MeshData.MinY ? VertexDataPointer->y : MeshData.MinY;
            MeshData.MaxY = VertexDataPointer->y > MeshData.MaxY ? VertexDataPointer->y : MeshData.MaxY;

            MeshData.MinZ = VertexDataPointer->z < MeshData.MinZ ? VertexDataPointer->z : MeshData.MinZ;
            MeshData.MaxZ = VertexDataPointer->z > MeshData.MaxZ ? VertexDataPointer->z : MeshData.MaxZ;

            VertexDataPointer->x = Mesh->mNormals[i].x;
            VertexDataPointer->y = Mesh->mNormals[i].y;
            VertexDataPointer->z = Mesh->mNormals[i].z;

            VertexDataPointer += 1;
            CombinedMesh.VertexData.Size += sizeof(glm::vec3);

            VertexDataPointer->x = Mesh->mTangents[i].x;
            VertexDataPointer->y = Mesh->mTangents[i].y;
            VertexDataPointer->z = Mesh->mTangents[i].z;

            VertexDataPointer += 1;
            CombinedMesh.VertexData.Size += sizeof(glm::vec3);

            glm::vec2* texPtr = (glm::vec2*)VertexDataPointer;

            texPtr->x = Mesh->mTextureCoords[0][i].x;
            texPtr->y = Mesh->mTextureCoords[0][i].y;

            CombinedMesh.VertexData.Size += sizeof(glm::vec2);
        }

        // Now wak through each of the mesh's faces (a face
        // in a mesh it's triangle) and retrieve the
        // corresponding vertex indices.
        for (unsigned int idx = 0; idx < Mesh->mNumFaces; ++idx)
        {
            aiFace* Face = Mesh->mFaces + idx;

            // Copy the face's indices to the element buffer
            u32*           ElementPtr   = (uint32_t*)(CombinedMesh.ElementData.Pointer + CombinedMesh.ElementData.Size);
            const uint32_t FaceDataSize = 3 * sizeof(uint32_t);

            ElementPtr[0] = (CurrentTotalElementCount + Face->mIndices[0]);
            ElementPtr[1] = (CurrentTotalElementCount + Face->mIndices[1]);
            ElementPtr[2] = (CurrentTotalElementCount + Face->mIndices[2]);

            CombinedMesh.ElementData.Size += FaceDataSize;
        }
    }

    static CTextureResource* AssimpImportMaterialTexture(const u8&      InIndex,
                                                         const FString& InMeshDirPath,
                                                         aiMaterial*    Material,
                                                         aiTextureType  TextureType,
                                                         const FString& MeshName,
                                                         const FString& TextureTypeName,
                                                         const bool&    InFlipUV)
    {
        // Import the texture
        aiString TextureFilePath;
        if (Material->GetTexture(TextureType, InIndex, &TextureFilePath) == aiReturn_FAILURE)
        {
            return nullptr;
        }
        std::filesystem::path Path{ TextureFilePath.C_Str() };
        Path.replace_extension("");

        FDString TexturePath             = SPrintf("%s/%s", *InMeshDirPath, TextureFilePath.C_Str());
        FDString TextureName             = SPrintf("%s_Texture_%s_%s", *MeshName, Path.filename().string().c_str(), *TextureTypeName);
        FDString TextureResourceFilePath = SPrintf("assets/textures/%s.asset", *TextureName);

        bool bGammaCorrect = false;
        switch (TextureType)
        {
        case aiTextureType_DIFFUSE:
        case aiTextureType_SPECULAR:
            bGammaCorrect = true;
            break;
        }

        CTextureResource* Texture =
          ImportTexture(TexturePath, TextureResourceFilePath, bGammaCorrect, gpu::ETextureDataType::UNSIGNED_BYTE, InFlipUV, false, TextureName);
        Texture->FreeMainMemory();

        // Update engine resources database
        GEngine.AddTextureResource(Texture);

        TexturePath.Free();

        if (Texture == nullptr)
        {
            TextureName.Free();
            LUCID_LOG(ELogLevel::WARN, "Failed to load %s texture of mesh %s", TextureTypeName, MeshName)
            return GEngine.GetTexturesHolder().GetDefaultResource();
        }

        // Save to a texture resource
        FILE* TextureResourceFile = fopen(*TextureResourceFilePath, "wb");
        if (TextureResourceFile == nullptr)
        {
            TextureResourceFilePath.Free();
            TextureName.Free();

            LUCID_LOG(ELogLevel::WARN,
                      "Failed to save save imported %d texture of "
                      "mesh %s - failed to open the file %s",
                      *TextureTypeName,
                      *MeshName,
                      *TextureResourceFilePath);
            return GEngine.GetTexturesHolder().GetDefaultResource();
        }

        Texture->SaveSynchronously(TextureResourceFile);
        fclose(TextureResourceFile);

        return Texture;
    }

    CMeshResource* LoadMesh(const FString& FilePath)
    {
        FILE* MeshFile;
        if (fopen_s(&MeshFile, *FilePath, "rb") != 0)
        {
            LUCID_LOG(ELogLevel::ERR, "Failed to mesh texture from file %s", *FilePath);
            return nullptr;
        }

        CMeshResource* MeshResource = resources::LoadResource<resources::CMeshResource>(MeshFile, FilePath);
        fclose(MeshFile);
        return MeshResource;
    }

    void CMeshResource::MigrateToLatestVersion()
    {
        if (AssetSerializationVersion == 0)
        {
            MinPosX = 0, MaxPosX = 0;
            MinPosY = 0, MaxPosY = 0;
            MinPosZ = 0, MaxPosZ = 0;
            AssetSerializationVersion = 1;
        }

        if (AssetSerializationVersion == 1)
        {
            DrawMode = gpu::EDrawMode::TRIANGLES;
        }
        
        Resave(MESH_SERIALIZATION_VERSION);
    }

} // namespace lucid::resources
