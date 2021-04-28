#include "resources/mesh_resource.hpp"



#include <filesystem>
#include <engine/engine.hpp>
#include <scene/actors/actor.hpp>


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
        // Read the size of vertex and element data
        fread_s(&VertexData.Capacity, sizeof(VertexData.Capacity), sizeof(VertexData.Capacity), 1, ResourceFile);
        fread_s(&ElementData.Capacity, sizeof(ElementData.Capacity), sizeof(ElementData.Capacity), 1, ResourceFile);
        fread_s(&VertexCount, sizeof(VertexCount), sizeof(VertexCount), 1, ResourceFile);
        fread_s(&ElementCount, sizeof(ElementCount), sizeof(ElementCount), 1, ResourceFile);

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
        u32 VertexDataOffset = Offset + RESOURCE_FILE_HEADER_SIZE + Name.GetLength() + sizeof(VertexData.Capacity) + sizeof(ElementData.Capacity) + sizeof(VertexCount) + sizeof(ElementCount);
        if (AssetSerializationVersion > 0)
        {
            VertexDataOffset += sizeof(float) * 6;
        }
        fseek(MeshFile, VertexDataOffset, SEEK_SET);

        // Read vertex data
        VertexData.Pointer = (char*)malloc(VertexData.Capacity);
        u64 NumElementsRead = fread_s(VertexData.Pointer, VertexData.Capacity, VertexData.Capacity, 1, MeshFile);
        assert(NumElementsRead == 1);
        VertexData.Size = VertexData.Capacity;

        // Read element data
        if (ElementData.Capacity > 0)
        {
            ElementData.Pointer = (char*)malloc(ElementData.Capacity);
            NumElementsRead = fread_s(ElementData.Pointer, ElementData.Capacity, ElementData.Capacity, 1, MeshFile);
            assert(NumElementsRead == 1);
            ElementData.Size = ElementData.Capacity;
        }

        // Close the file
        fclose(MeshFile);
    }

    void CMeshResource::LoadDataToVideoMemorySynchronously()
    {
        assert(VertexData.Pointer);
        // Check if we didn't load it already
        if (VertexBuffer)
        {
            return;
        }

        gpu::FBufferDescription GPUBufferDescription;

        // Sending vertex data to the gpu
        GPUBufferDescription.Data = VertexData.Pointer;
        GPUBufferDescription.Size = VertexData.Size;

        VertexBuffer = gpu::CreateBuffer(GPUBufferDescription, gpu::EBufferUsage::STATIC,
                                         SPrintf("%s_VertexBuffer", *Name));
        assert(VertexBuffer);

        // Sending element to the gpu if it's present
        if (ElementData.Pointer)
        {
            GPUBufferDescription.Data = ElementData.Pointer;
            GPUBufferDescription.Size = ElementData.Size;

            ElementBuffer = gpu::CreateBuffer(GPUBufferDescription, gpu::EBufferUsage::STATIC,
                                              SPrintf("%s_ElementBuffer", *Name));
            assert(ElementBuffer);
        }

        FArray<gpu::FVertexAttribute> MeshAttributes(4);

        // Position
        MeshAttributes.Add({ 0, 3, EType::FLOAT, false, sizeof(float) * 11, 0, 0});
        // Normal
        MeshAttributes.Add({ 1, 3, EType::FLOAT, false, sizeof(float) * 11, sizeof(float) * 3, 0});
        // Tangent
        MeshAttributes.Add({ 2, 3, EType::FLOAT, false, sizeof(float) * 11, sizeof(float) * 6, 0});
        // UV
        MeshAttributes.Add({ 3, 2, EType::FLOAT, false, sizeof(float) * 11, sizeof(float) * 9, 0});

        VAO = gpu::CreateVertexArray(SPrintf("%s_VAO", *Name), &MeshAttributes, VertexBuffer, ElementBuffer, gpu::EDrawMode::TRIANGLES, VertexCount, ElementCount);
        assert(VAO);

        MeshAttributes.Free();
    }

    void CMeshResource::SaveSynchronously(FILE* ResourceFile) const
    {
        assert(VertexData.Pointer);

        // Write header
        SaveHeader(ResourceFile);

        // Save size of vertex and element buffer
        fwrite(&VertexData.Capacity, sizeof(VertexData.Capacity), 1, ResourceFile);
        fwrite(&ElementData.Capacity, sizeof(ElementData.Capacity), 1, ResourceFile);
        fwrite(&VertexCount, sizeof(VertexCount), 1, ResourceFile);
        fwrite(&ElementCount, sizeof(ElementCount), 1, ResourceFile);

        fwrite(&MinPosX, sizeof(MinPosX), 1, ResourceFile);
        fwrite(&MaxPosX, sizeof(MaxPosX), 1, ResourceFile);
        fwrite(&MinPosY, sizeof(MinPosY), 1, ResourceFile);
        fwrite(&MaxPosY, sizeof(MaxPosY), 1, ResourceFile);
        fwrite(&MinPosZ, sizeof(MinPosZ), 1, ResourceFile);
        fwrite(&MaxPosZ, sizeof(MaxPosZ), 1, ResourceFile);

        // Save vertex data
        fwrite(VertexData.Pointer, VertexData.Size, 1, ResourceFile);

        // Save element data if present
        if (ElementData.Pointer)
        {
            fwrite(ElementData.Pointer, ElementData.Size, 1, ResourceFile);
        }
    }

    void CMeshResource::FreeMainMemory()
    {
        if (!IsMainMemoryFreed)
        {
            IsMainMemoryFreed = true;
            free(VertexData.Pointer);
            free(ElementData.Pointer);
        }
    }

    void CMeshResource::FreeVideoMemory()
    {
        if (!IsVideoMemoryFreed)
        {
            IsVideoMemoryFreed = true;
            VertexBuffer->Free();
            ElementBuffer->Free();
            VAO->Free();
        }
    }

    CResourcesHolder<CMeshResource> MeshesHolder;

    static Assimp::Importer AssimpImporter;

    static const constexpr u32 ASSIMP_DEFAULT_FLAGS = aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_OptimizeMeshes;

    /* Helper structure used when importing the mesh */
    struct FMeshSize
    {
        u32 VertexDataSize = 0;
        u32 ElementDataSize = 0;
    };

    /* Helper structure used when importing the mesh */
    struct FMeshInfoHelper
    {
        FMemBuffer VertexBuffer;
        FMemBuffer ElementBuffer;
        u32 VertexCount = 0;
        u32 ElementCount = 0;
        float MinX, MaxX;
        float MinY, MaxY;
        float MinZ, MaxZ;
    };

    static FMeshSize AssimpCalculateMeshDataSize(aiNode* Node, const aiScene* Scene);

    static void LoadAssimpNode(aiNode* Node, const aiScene* Scene, FMeshInfoHelper& MeshData);
    static void LoadAssimpMesh(aiMesh* mesh, const aiScene* scene, FMeshInfoHelper& MeshData);
    
    static CTextureResource* AssimpImportMaterialTexture(const FString& ModelFileDir, aiMaterial* Material, aiTextureType TextureType, const FString& MeshName, const FString& TextureTypeName);

    CMeshResource* ImportMesh(const FString& InMeshFilePath, const FString& InMeshResourceFilePath, const FString& MeshName)
    {
#ifndef NDEBUG
        real StartTime = platform::GetCurrentTimeSeconds();
#endif

        const aiScene* Root = AssimpImporter.ReadFile(*InMeshFilePath, ASSIMP_DEFAULT_FLAGS);

        LUCID_LOG(ELogLevel::INFO, "Reading mesh with assimp %s took %f", *InMeshFilePath, platform::GetCurrentTimeSeconds() - StartTime);

        if (!Root || Root->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Root->mRootNode)
        {
            LUCID_LOG(ELogLevel::WARN, "Assimp failed to load model %s", AssimpImporter.GetErrorString())
            return nullptr;
        }

        // Allocate main memory needed for the mesh
        const FMeshSize MeshDataSize = AssimpCalculateMeshDataSize(Root->mRootNode, Root);

        const FMemBuffer VertexDataBuffer = CreateMemBuffer(MeshDataSize.VertexDataSize);
        const FMemBuffer ElementDataBuffer = MeshDataSize.ElementDataSize > 0
                                                 ? CreateMemBuffer(MeshDataSize.ElementDataSize)
                                                 : FMemBuffer{nullptr, 0, 0};

        FMeshInfoHelper MeshInfoHelper{VertexDataBuffer, ElementDataBuffer, 0, 0};

        // Load the assimp nodes recursively

        StartTime = platform::GetCurrentTimeSeconds();

        MeshInfoHelper.MinX = MeshInfoHelper.MinY = MeshInfoHelper.MinZ = FLT_MAX;
        MeshInfoHelper.MaxX = MeshInfoHelper.MaxY = MeshInfoHelper.MaxZ = 0;

        LoadAssimpNode(Root->mRootNode, Root, MeshInfoHelper);

        // Load textures
        aiMaterial* Material = Root->mMaterials[1];
        std::filesystem::path MeshFilePath { *InMeshFilePath };


        FDString MeshFileDirPath = CopyToString(MeshFilePath.parent_path().string().c_str()); // @TODO .string().c_str() once we support wchars
        StartTime = platform::GetCurrentTimeSeconds();

        if (Material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            AssimpImportMaterialTexture(MeshFileDirPath, Material, aiTextureType_DIFFUSE,  MeshName, FString { "Diffuse" } );
        }

        if (Material->GetTextureCount(aiTextureType_SPECULAR))
        {
            AssimpImportMaterialTexture(MeshFileDirPath, Material, aiTextureType_SPECULAR, MeshName, FString { "Specular" } );
        }

        if (Material->GetTextureCount(aiTextureType_HEIGHT))
        {
            AssimpImportMaterialTexture(MeshFileDirPath, Material, aiTextureType_HEIGHT, MeshName, FString { "Normal" } );
        }

        MeshFileDirPath.Free();

#ifndef NDEBUG
        LUCID_LOG(ELogLevel::INFO, "Loading textures of mesh %s took %f", *MeshName, platform::GetCurrentTimeSeconds() - StartTime);
        LUCID_LOG(ELogLevel::INFO, "Sending mesh %s data to GPU took %f", *MeshName,platform::GetCurrentTimeSeconds() - StartTime);
#endif

        auto* ImportedMesh = new CMeshResource { sole::uuid4(), MeshName, InMeshResourceFilePath, 0, MeshDataSize.VertexDataSize + MeshDataSize.ElementDataSize, MESH_SERIALIZATION_VERSION };

        ImportedMesh->VertexData = MeshInfoHelper.VertexBuffer;
        ImportedMesh->ElementData = MeshInfoHelper.ElementBuffer;

        ImportedMesh->VertexCount = MeshInfoHelper.VertexCount;
        ImportedMesh->ElementCount = MeshInfoHelper.ElementCount;

        ImportedMesh->MinPosX = MeshInfoHelper.MinX;
        ImportedMesh->MaxPosX = MeshInfoHelper.MaxX;

        ImportedMesh->MinPosY = MeshInfoHelper.MinY;
        ImportedMesh->MaxPosY = MeshInfoHelper.MaxY;

        ImportedMesh->MinPosZ = MeshInfoHelper.MinZ;
        ImportedMesh->MaxPosZ = MeshInfoHelper.MaxZ;        
        
        return ImportedMesh;
    }; // namespace lucid::resources

    static FMeshSize AssimpCalculateMeshDataSize(aiNode* Node, const aiScene* Scene)
    {
        FMeshSize MeshSize;
        for (u32 idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            aiMesh* MeshNode = Scene->mMeshes[Node->mMeshes[idx]];
            u32 vertexSize = 0;

            //Position
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
            const FMeshSize SubMeshSize = AssimpCalculateMeshDataSize(Node->mChildren[idx], Scene);
            MeshSize.VertexDataSize += SubMeshSize.VertexDataSize;
            MeshSize.ElementDataSize += SubMeshSize.ElementDataSize;
        }

        return MeshSize;
    }

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
        uint32_t CurrentTotalElementCount = MeshData.VertexCount;

        MeshData.VertexCount += Mesh->mNumVertices;
        for (uint32_t i = 0; i < Mesh->mNumVertices; ++i)
        {
            glm::vec3* VertexDataPointer = (glm::vec3*)(MeshData.VertexBuffer.Pointer + MeshData.VertexBuffer.Size);

            // Position
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

            MeshData.VertexBuffer.Size += sizeof(glm::vec3);

            // Normal
            VertexDataPointer->x = Mesh->mNormals[i].x;
            VertexDataPointer->y = Mesh->mNormals[i].y;
            VertexDataPointer->z = Mesh->mNormals[i].z;
            VertexDataPointer += 1;
            MeshData.VertexBuffer.Size += sizeof(glm::vec3);

            // Tangents
            VertexDataPointer->x = Mesh->mTangents[i].x;
            VertexDataPointer->y = Mesh->mTangents[i].y;
            VertexDataPointer->z = Mesh->mTangents[i].z;

            VertexDataPointer += 1;
            MeshData.VertexBuffer.Size += sizeof(glm::vec3);

            // UV 
            glm::vec2* texPtr = (glm::vec2*)VertexDataPointer;

            texPtr->x = Mesh->mTextureCoords[0][i].x;
            texPtr->y = Mesh->mTextureCoords[0][i].y;
            VertexDataPointer = (glm::vec3*)(texPtr + 1);
            MeshData.VertexBuffer.Size += sizeof(glm::vec2);
        }

        // Now wak through each of the mesh's faces (a face in a mesh it's triangle) and retrieve the corresponding vertex indices.
        for (unsigned int idx = 0; idx < Mesh->mNumFaces; ++idx)
        {
            aiFace* Face = Mesh->mFaces + idx;

            // Copy the face's indices to the element buffer
            u32*            ElementPtr = (uint32_t*)(MeshData.ElementBuffer.Pointer + MeshData.ElementBuffer.Size);
            const uint32_t  FaceDataSize = 3 * sizeof(uint32_t);

            ElementPtr[0] = (CurrentTotalElementCount + Face->mIndices[0]);
            ElementPtr[1] = (CurrentTotalElementCount + Face->mIndices[1]);
            ElementPtr[2] = (CurrentTotalElementCount + Face->mIndices[2]);

            MeshData.ElementCount += 3;
            MeshData.ElementBuffer.Size += FaceDataSize;
        }
    }

    static CTextureResource* AssimpImportMaterialTexture(const FString& InMeshDirPath, aiMaterial* Material, aiTextureType TextureType, const FString& MeshName, const FString& TextureTypeName)
    {
        // Import the texture
        aiString TextureFilePath;
        Material->GetTexture(TextureType, 0, &TextureFilePath);

        FDString TexturePath = SPrintf("%s/%s", *InMeshDirPath, TextureFilePath.C_Str());
        FDString TextureName = SPrintf("%s_Texture%s", *MeshName, *TextureTypeName);
        FDString TextureResourceFilePath =   SPrintf("assets/textures/%s.asset", *TextureName);

        CTextureResource* Texture = ImportJPGTexture(TexturePath, TextureResourceFilePath, true, gpu::ETextureDataType::UNSIGNED_BYTE, true, false, SPrintf("%s_%s", *MeshName, *TextureTypeName));
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
        FILE*       TextureResourceFile =       fopen(*TextureResourceFilePath, "wb");
        if (TextureResourceFile == nullptr)
        {
            TextureResourceFilePath.Free();
            TextureName.Free();

            LUCID_LOG(ELogLevel::WARN, "Failed to save save imported %d texture of mesh %s - failed to open the file", *TextureTypeName, *MeshName);
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
        if(fopen_s(&MeshFile, *FilePath, "rb") != 0)
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
