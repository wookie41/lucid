#include "resources/mesh_resource.hpp"

#include <filesystem>
#include "engine/engine.hpp"
#include "scene/actors/actor.hpp"

#include "common/log.hpp"
#include "common/bytes.hpp"

#include "glm/glm.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "devices/gpu/buffer.hpp"
#include "resources/texture_resource.hpp"
#include "platform/util.hpp"
#include "devices/gpu/texture_enums.hpp"
#include "resources/serialization_versions.hpp"

#include "schemas/types.hpp"

namespace lucid::resources
{
#define SUBMESH_INFO_SIZE (((sizeof(u32) * 4) + (sizeof(bool) * 4)))

    CMeshResource::CMeshResource(const UUID& InID,
                                 const FString& InName,
                                 const FString& InFilePath,
                                 const u64& InOffset,
                                 const u64& InDataSize,
                                 const u32& InAssetSerializationVersion)
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

            // SubMesh.VertexDataBuffer.Size = SubMesh.VertexDataBuffer.Capacity;
            // SubMesh.ElementDataBuffer.Size = SubMesh.ElementDataBuffer.Capacity;

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
    }

    void CMeshResource::LoadDataToMainMemorySynchronously()
    {
        FILE* MeshFile;
        if (fopen_s(&MeshFile, *FilePath, "rb") != 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to open file %s", *FilePath);
            return;
        }
        // Position the file pointer to the beginning of mesh data
        u32 VertexDataOffset = Offset + RESOURCE_FILE_HEADER_SIZE + Name.GetLength() + sizeof(u16) + (SUBMESH_INFO_SIZE * SubMeshes.GetLength());
        if (AssetSerializationVersion > 0)
        {
            VertexDataOffset += sizeof(float) * 6;
        }
        fseek(MeshFile, VertexDataOffset, SEEK_SET);

        // Read vertex and element data for each submesh
        for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
        {
            FSubMesh* SubMesh = SubMeshes[i];

            SubMesh->VertexDataBuffer.Pointer = (char*)malloc(SubMesh->VertexDataBuffer.Capacity);
            u64 NumElementsRead = fread_s(SubMesh->VertexDataBuffer.Pointer,
                                          SubMesh->VertexDataBuffer.Capacity,
                                          SubMesh->VertexDataBuffer.Capacity,
                                          1,
                                          MeshFile);
            assert(NumElementsRead == 1);
            SubMesh->VertexDataBuffer.Size = SubMesh->VertexDataBuffer.Capacity;

            // Read element data
            if (SubMesh->ElementDataBuffer.Capacity > 0)
            {
                SubMesh->ElementDataBuffer.Pointer = (char*)malloc(SubMesh->ElementDataBuffer.Capacity);
                NumElementsRead = fread_s(SubMesh->ElementDataBuffer.Pointer,
                                          SubMesh->ElementDataBuffer.Capacity,
                                          SubMesh->ElementDataBuffer.Capacity,
                                          1,
                                          MeshFile);
                assert(NumElementsRead == 1);
                SubMesh->ElementDataBuffer.Size = SubMesh->ElementDataBuffer.Capacity;

            }
        }

        // Close the file
        fclose(MeshFile);
    }

    void CMeshResource::LoadDataToVideoMemorySynchronously()
    {
        // Check if we didn't load it already
        if (SubMeshes.GetLength() && SubMeshes[0]->VAO)
        {
            return;
        }

        for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
        {
            FSubMesh* SubMesh = SubMeshes[i];

            gpu::FBufferDescription GPUBufferDescription;

            // Sending vertex data to the gpu
            GPUBufferDescription.Data = SubMesh->VertexDataBuffer.Pointer;
            GPUBufferDescription.Size = SubMesh->VertexDataBuffer.Size;

            SubMesh->VertexBuffer = gpu::CreateBuffer(GPUBufferDescription, gpu::EBufferUsage::STATIC, SPrintf("%s_VertexBuffer_%d", *Name, i));
            assert(SubMesh->VertexBuffer);
            // Sending element to the gpu if it's present
            if (SubMesh->ElementDataBuffer.Pointer)
            {
                GPUBufferDescription.Data = SubMesh->ElementDataBuffer.Pointer;
                GPUBufferDescription.Size = SubMesh->ElementDataBuffer.Size;

                SubMesh->ElementBuffer = gpu::CreateBuffer(GPUBufferDescription, gpu::EBufferUsage::STATIC, SPrintf("%s_ElementBuffer_%d", *Name, i));
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
                                                  gpu::EDrawMode::TRIANGLES,
                                                  SubMesh->VertexCount,
                                                  SubMesh->ElementCount);
            assert(SubMesh->VAO);

            MeshAttributes.Free();
        }
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
        }

        fwrite(&MinPosX, sizeof(MinPosX), 1, ResourceFile);
        fwrite(&MaxPosX, sizeof(MaxPosX), 1, ResourceFile);
        fwrite(&MinPosY, sizeof(MinPosY), 1, ResourceFile);
        fwrite(&MaxPosY, sizeof(MaxPosY), 1, ResourceFile);
        fwrite(&MinPosZ, sizeof(MinPosZ), 1, ResourceFile);
        fwrite(&MaxPosZ, sizeof(MaxPosZ), 1, ResourceFile);

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
        if (!IsMainMemoryFreed)
        {
            for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
            {
                free(SubMeshes[i]->VertexDataBuffer.Pointer);
                free(SubMeshes[i]->ElementDataBuffer.Pointer);
            }

            IsMainMemoryFreed = true;
        }
    }

    void CMeshResource::FreeVideoMemory()
    {
        if (!IsVideoMemoryFreed)
        {
            for (u16 i = 0; i < SubMeshes.GetLength(); ++i)
            {
                SubMeshes[i]->VertexBuffer->Free();
                SubMeshes[i]->ElementBuffer->Free();
            }

            IsVideoMemoryFreed = true;
        }
    }

    CResourcesHolder<CMeshResource> MeshesHolder;

    static Assimp::Importer AssimpImporter;

    static constexpr u32 ASSIMP_DEFAULT_FLAGS = aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                                                aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices;

    /* Helper structure used when importing the mesh */
    struct FSubMeshInfo
    {
        bool bHasPositions = false;
        bool bHasNormals = false;
        bool bHasTangents = false;
        bool bHasUVs = false;

        FMemBuffer VertexData;
        FMemBuffer ElementData;

        u32 VertexCount = 0;
        u32 ElementCount = 0;
    };

    /* Helper structure used when importing the mesh */
    struct FMeshInfoHelper
    {
        FArray<FSubMeshInfo> SubMeshes{ 1, true };

        float MinX = 0, MaxX = 0;
        float MinY = 0, MaxY = 0;
        float MinZ = 0, MaxZ = 0;
    };

    static void LoadAssimpNode(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData);
    static void LoadAssimpMesh(aiMesh* mesh, const aiScene* scene, FMeshInfoHelper& MeshData);

    static CTextureResource* AssimpImportMaterialTexture(const u8& InIndex,
                                                         const FString& ModelFileDir,
                                                         aiMaterial* Material,
                                                         aiTextureType TextureType,
                                                         const FString& MeshName,
                                                         const FString& TextureTypeName);

    CMeshResource* ImportMesh(const FString& InMeshFilePath, const FString& InMeshResourceFilePath, const FString& MeshName)
    {
#ifndef NDEBUG
        real StartTime = platform::GetCurrentTimeSeconds();
#endif

        const aiScene* Root = AssimpImporter.ReadFile(*InMeshFilePath, ASSIMP_DEFAULT_FLAGS);

        LUCID_LOG(
          ELogLevel::INFO, "Reading mesh with assimp %s took %f", *InMeshFilePath, platform::GetCurrentTimeSeconds() - StartTime);

        if (!Root || Root->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Root->mRootNode)
        {
            LUCID_LOG(ELogLevel::WARN, "Assimp failed to load model %s", AssimpImporter.GetErrorString())
            return nullptr;
        }

        FMeshInfoHelper MeshInfoHelper;

        // Load the assimp nodes recursively
        StartTime = platform::GetCurrentTimeSeconds();

        MeshInfoHelper.MinX = MeshInfoHelper.MinY = MeshInfoHelper.MinZ = FLT_MAX;
        MeshInfoHelper.MaxX = MeshInfoHelper.MaxY = MeshInfoHelper.MaxZ = 0;

        LoadAssimpNode(Root->mRootNode, Root, MeshInfoHelper);

        // Load textures
        std::filesystem::path MeshFilePath{ *InMeshFilePath };
        for (int MaterialIndex = 0; MaterialIndex < Root->mNumMaterials; ++MaterialIndex)
        {
            aiMaterial* Material = Root->mMaterials[MaterialIndex];
            FDString MeshFileDirPath =
              CopyToString(MeshFilePath.parent_path().string().c_str()); // @TODO .string().c_str() once we support wchars
            StartTime = platform::GetCurrentTimeSeconds();

            for (int TextureIndex = 0; TextureIndex < Material->GetTextureCount(aiTextureType_DIFFUSE); ++TextureIndex)
            {
                AssimpImportMaterialTexture(
                  TextureIndex, MeshFileDirPath, Material, aiTextureType_DIFFUSE, MeshName, FString{ "Diffuse" });
            }

            for (int TextureIndex = 0; TextureIndex < Material->GetTextureCount(aiTextureType_SPECULAR); ++TextureIndex)
            {
                AssimpImportMaterialTexture(
                  TextureIndex, MeshFileDirPath, Material, aiTextureType_SPECULAR, MeshName, FString{ "Specular" });
            }

            for (int TextureIndex = 0; TextureIndex < Material->GetTextureCount(aiTextureType_HEIGHT); ++TextureIndex)
            {
                AssimpImportMaterialTexture(
                  TextureIndex, MeshFileDirPath, Material, aiTextureType_HEIGHT, MeshName, FString{ "Normal" });
            }

            MeshFileDirPath.Free();
        }

#ifndef NDEBUG
        LUCID_LOG(
          ELogLevel::INFO, "Loading textures of mesh %s took %f", *MeshName, platform::GetCurrentTimeSeconds() - StartTime);
        LUCID_LOG(
          ELogLevel::INFO, "Sending mesh %s data to GPU took %f", *MeshName, platform::GetCurrentTimeSeconds() - StartTime);
#endif

        u64 TotalDataSize = 0;
        for (u16 i = 0; i < MeshInfoHelper.SubMeshes.GetLength(); ++i)
        {
            TotalDataSize += MeshInfoHelper.SubMeshes[i]->VertexData.Size;
            TotalDataSize += MeshInfoHelper.SubMeshes[i]->ElementData.Size;
        }

        auto* ImportedMesh =
          new CMeshResource{ sole::uuid4(), MeshName, InMeshResourceFilePath, 0, TotalDataSize, MESH_SERIALIZATION_VERSION };

        for (u16 i = 0; i < MeshInfoHelper.SubMeshes.GetLength(); ++i)
        {
            FSubMesh SubMesh;
            SubMesh.bHasPositions = MeshInfoHelper.SubMeshes[i]->bHasPositions;
            SubMesh.bHasNormals = MeshInfoHelper.SubMeshes[i]->bHasNormals;
            SubMesh.bHasTangetns = MeshInfoHelper.SubMeshes[i]->bHasTangents;
            SubMesh.bHasUVs = MeshInfoHelper.SubMeshes[i]->bHasUVs;
            SubMesh.VertexDataBuffer = MeshInfoHelper.SubMeshes[i]->VertexData;
            SubMesh.ElementDataBuffer = MeshInfoHelper.SubMeshes[i]->ElementData;
            SubMesh.VertexCount = MeshInfoHelper.SubMeshes[i]->VertexCount;
            SubMesh.ElementCount = MeshInfoHelper.SubMeshes[i]->ElementCount;
            ImportedMesh->SubMeshes.Add(SubMesh);
        }

        ImportedMesh->MinPosX = MeshInfoHelper.MinX;
        ImportedMesh->MaxPosX = MeshInfoHelper.MaxX;

        ImportedMesh->MinPosY = MeshInfoHelper.MinY;
        ImportedMesh->MaxPosY = MeshInfoHelper.MaxY;

        ImportedMesh->MinPosZ = MeshInfoHelper.MinZ;
        ImportedMesh->MaxPosZ = MeshInfoHelper.MaxZ;

        return ImportedMesh;
    }; // namespace lucid::resources

    static void LoadAssimpNode(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData)
    {
        // process each mesh located at the current node
        for (u32 idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            LoadAssimpMesh(mesh, Scene, MeshData);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (u32 idx = 0; idx < Node->mNumChildren; ++idx)
        {
            LoadAssimpNode(Node->mChildren[idx], Scene, MeshData);
        }
    }

    void LoadAssimpMesh(aiMesh* Mesh, const aiScene* Scene, FMeshInfoHelper& MeshData)
    {
        FSubMeshInfo SubMeshInfo;
        SubMeshInfo.VertexCount = Mesh->mNumVertices;
        SubMeshInfo.ElementCount = Mesh->mNumFaces * 3;

        u16 VertexSize = 0;
        if (Mesh->HasPositions())
        {
            VertexSize += sizeof(glm::vec3);
            SubMeshInfo.bHasPositions = true;
        }

        if (Mesh->HasNormals())
        {
            VertexSize += sizeof(glm::vec3);
            SubMeshInfo.bHasNormals = true;
        }
        // Normals
        // Tangents
        if (Mesh->HasTangentsAndBitangents())
        {
            VertexSize += sizeof(glm::vec3);
            SubMeshInfo.bHasTangents = true;
        }

        if (Mesh->HasTextureCoords(0))
        {
            VertexSize += sizeof(glm::vec2);
            SubMeshInfo.bHasUVs = true;
        }

        // UV
        const u32 VertexDataSize = Mesh->mNumVertices * VertexSize;
        const u32 ElementDataSize = (Mesh->mNumFaces * 3 * sizeof(u32));

        SubMeshInfo.VertexData = CreateMemBuffer(VertexDataSize);
        SubMeshInfo.ElementData = ElementDataSize > 0 ? CreateMemBuffer(ElementDataSize) : FMemBuffer{};        

        for (uint32_t i = 0; i < Mesh->mNumVertices; ++i)
        {
            glm::vec3* VertexDataPointer = (glm::vec3*)(SubMeshInfo.VertexData.Pointer + SubMeshInfo.VertexData.Size);

            // @TODO Extract the bHasXXX out of the loop to avoid so many checks

            // Position
            if (SubMeshInfo.bHasPositions)
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
                SubMeshInfo.VertexData.Size += sizeof(glm::vec3);
            }

            if (SubMeshInfo.bHasNormals)
            {
                VertexDataPointer->x = Mesh->mNormals[i].x;
                VertexDataPointer->y = Mesh->mNormals[i].y;
                VertexDataPointer->z = Mesh->mNormals[i].z;
                VertexDataPointer += 1;
                SubMeshInfo.VertexData.Size += sizeof(glm::vec3);
            }

            if (SubMeshInfo.bHasTangents)
            {
                VertexDataPointer->x = Mesh->mTangents[i].x;
                VertexDataPointer->y = Mesh->mTangents[i].y;
                VertexDataPointer->z = Mesh->mTangents[i].z;

                VertexDataPointer += 1;
                SubMeshInfo.VertexData.Size += sizeof(glm::vec3);
            }

            if (SubMeshInfo.bHasUVs)
            {
                glm::vec2* texPtr = (glm::vec2*)VertexDataPointer;

                texPtr->x = Mesh->mTextureCoords[0][i].x;
                texPtr->y = Mesh->mTextureCoords[0][i].y;
                VertexDataPointer = (glm::vec3*)(texPtr + 1);
                SubMeshInfo.VertexData.Size += sizeof(glm::vec2);
            }
        }

        // Now wak through each of the mesh's faces (a face in a mesh it's triangle) and retrieve the corresponding vertex indices.
        for (unsigned int idx = 0; idx < Mesh->mNumFaces; ++idx)
        {
            aiFace* Face = Mesh->mFaces + idx;

            // Copy the face's indices to the element buffer
            u32* ElementPtr = (uint32_t*)(SubMeshInfo.ElementData.Pointer + SubMeshInfo.ElementData.Size);
            const uint32_t FaceDataSize = 3 * sizeof(uint32_t);

            ElementPtr[0] = Face->mIndices[0];
            ElementPtr[1] = Face->mIndices[1];
            ElementPtr[2] = Face->mIndices[2];

            SubMeshInfo.ElementData.Size += FaceDataSize;
        }

        MeshData.SubMeshes.Add(SubMeshInfo);
    }

    static CTextureResource* AssimpImportMaterialTexture(const u8& InIndex,
                                                         const FString& InMeshDirPath,
                                                         aiMaterial* Material,
                                                         aiTextureType TextureType,
                                                         const FString& MeshName,
                                                         const FString& TextureTypeName)
    {
        // Import the texture
        aiString TextureFilePath;
        Material->GetTexture(TextureType, InIndex, &TextureFilePath);
        std::filesystem::path Path{ TextureFilePath.C_Str() };
        Path.replace_extension("");

        FDString TexturePath = SPrintf("%s/%s", *InMeshDirPath, TextureFilePath.C_Str());
        FDString TextureName = SPrintf("%s_Texture_%s", *MeshName, Path.string().c_str());
        FDString TextureResourceFilePath = SPrintf("assets/textures/%s.asset", *TextureName);

        CTextureResource* Texture = ImportTexture(
          TexturePath, TextureResourceFilePath, true, gpu::ETextureDataType::UNSIGNED_BYTE, true, false, TextureName);
        Texture->LoadDataToVideoMemorySynchronously();

        // Update engine resources database
        GEngine.AddTextureResource(Texture, TexturePath);

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
                      "Failed to save save imported %d texture of mesh %s - failed to open the file %s",
                      *TextureTypeName,
                      *MeshName,
                      *TextureResourceFilePath);
            return GEngine.GetTexturesHolder().GetDefaultResource();
        }

        Texture->SaveSynchronously(TextureResourceFile);
        fclose(TextureResourceFile);

        TextureResourceFilePath.Free();
        TextureName.Free();

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

        Resave();
    }

} // namespace lucid::resources
