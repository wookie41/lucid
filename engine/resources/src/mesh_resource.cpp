#include "resources/mesh_resource.hpp"

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

namespace lucid::resources
{
    
    CResourcesHolder<CTextureResource> TexturesHolder{};
    ;

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
        fseek(MeshFile,Offset + RESOURCE_FILE_HEADER_SIZE + Name.GetLength() + sizeof(VertexData.Capacity) + sizeof(ElementData.Capacity) + sizeof(VertexCount) + sizeof(ElementCount), SEEK_SET);

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

    void CMeshResource::SaveSynchronously(FILE* ResourceFile)
    {
        assert(VertexData.Pointer);

        // Set version of serialization code
        AssetSerializationVersion = MESH_SERIALIZATION_VERSION;

        // Write header
        SaveHeader(ResourceFile);

        // Save size of vertex and element buffer
        fwrite(&VertexData.Capacity, sizeof(VertexData.Capacity), 1, ResourceFile);
        fwrite(&ElementData.Capacity, sizeof(ElementData.Capacity), 1, ResourceFile);
        fwrite(&VertexCount, sizeof(VertexCount), 1, ResourceFile);
        fwrite(&ElementCount, sizeof(ElementCount), 1, ResourceFile);

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
    struct FMeshMainMemoryBuffers
    {
        FMemBuffer VertexBuffer;
        FMemBuffer ElementBuffer;
        u32 VertexCount = 0;
        u32 ElementCount = 0;
    };

    static FMeshSize AssimpCalculateMeshDataSize(aiNode* Node, const aiScene* Scene);

    static void LoadAssimpNode(const FString& DirectoryPath, aiNode* Node, const aiScene* Scene, FMeshMainMemoryBuffers& MeshData);
    static void LoadAssimpMesh(const FString& DirectoryPath, aiMesh* mesh, const aiScene* scene, FMeshMainMemoryBuffers& MeshData);
    
    static CTextureResource* AssimpImportMaterialTexture(const FString& DirectoryPath, aiMaterial* Material, aiTextureType TextureType, const FString& MeshName, const FString& TextureTypeName);

    CMeshResource* ImportMesh(const FString& DirectoryPath, const FString& MeshFileName, const FString& MeshName)
    {
        // Read mesh file
        FDString MeshFilePath = SPrintf("%s/%s", *DirectoryPath, *MeshFileName);

#ifndef NDEBUG
        real StartTime = platform::GetCurrentTimeSeconds();
#endif

        const aiScene* Root = AssimpImporter.ReadFile(*MeshFilePath, ASSIMP_DEFAULT_FLAGS);

        LUCID_LOG(ELogLevel::INFO, "Reading mesh with assimp %s took %f", *MeshFileName, platform::GetCurrentTimeSeconds() - StartTime);

        if (!Root || Root->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Root->mRootNode)
        {
            LUCID_LOG(ELogLevel::WARN, "Assimp failed to load model %s", AssimpImporter.GetErrorString())
            MeshFilePath.Free();
            return nullptr;
        }

        // Allocate main memory needed for the mesh
        const FMeshSize MeshDataSize = AssimpCalculateMeshDataSize(Root->mRootNode, Root);

        const FMemBuffer VertexDataBuffer = CreateMemBuffer(MeshDataSize.VertexDataSize);
        const FMemBuffer ElementDataBuffer = MeshDataSize.ElementDataSize > 0
                                                 ? CreateMemBuffer(MeshDataSize.ElementDataSize)
                                                 : FMemBuffer{nullptr, 0, 0};

        FMeshMainMemoryBuffers MainMemoryBuffers{VertexDataBuffer, ElementDataBuffer, 0, 0};

        // Load the assimp nodes recursively

        StartTime = platform::GetCurrentTimeSeconds();

        LoadAssimpNode(DirectoryPath, Root->mRootNode, Root, MainMemoryBuffers);

        LUCID_LOG(ELogLevel::INFO, "Vertex data  %u/%u, element data %u/%u", MainMemoryBuffers.VertexBuffer.Size,
                  MainMemoryBuffers .VertexBuffer.Capacity, MainMemoryBuffers.ElementBuffer.Size,
                  MainMemoryBuffers.ElementBuffer.Capacity);

#ifndef NDEBUG
        LUCID_LOG(ELogLevel::INFO, "Loading mesh %s took %f", *MeshFileName,
                  platform::GetCurrentTimeSeconds() - StartTime);
#endif

        // Load textures
        aiMaterial* Material = Root->mMaterials[1];

        StartTime = platform::GetCurrentTimeSeconds();

        if (Material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            AssimpImportMaterialTexture(DirectoryPath, Material, aiTextureType_DIFFUSE,  MeshName, FString { "Diffuse" } );
        }

        if (Material->GetTextureCount(aiTextureType_SPECULAR))
        {
            AssimpImportMaterialTexture(DirectoryPath, Material, aiTextureType_SPECULAR, MeshName, FString { "Specular" } );
        }

        if (Material->GetTextureCount(aiTextureType_HEIGHT))
        {
            AssimpImportMaterialTexture(DirectoryPath, Material, aiTextureType_HEIGHT, MeshName, FString { "Normal" } );
        }

#ifndef NDEBUG
        LUCID_LOG(ELogLevel::INFO, "Loading textures %s took %f", *MeshFileName, platform::GetCurrentTimeSeconds() - StartTime);
#endif
        
        MeshFilePath.Free();

#ifndef NDEBUG

        LUCID_LOG(ELogLevel::INFO, "Sending mesh %s data to GPU took %f", *MeshFileName,
                  platform::GetCurrentTimeSeconds() - StartTime);
#endif

        auto* ImportedMesh = new CMeshResource { sole::uuid4(), MeshName, FString { "" }, 0, MeshDataSize.VertexDataSize + MeshDataSize.ElementDataSize, MESH_SERIALIZATION_VERSION };

        ImportedMesh->VertexData = MainMemoryBuffers.VertexBuffer;
        ImportedMesh->ElementData = MainMemoryBuffers.ElementBuffer;

        ImportedMesh->VertexCount = MainMemoryBuffers.VertexCount;
        ImportedMesh->ElementCount = MainMemoryBuffers.ElementCount;

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

    static void LoadAssimpNode(const FString& DirectoryPath, aiNode* Node, const aiScene* Scene,
                               FMeshMainMemoryBuffers& MeshData)
    {
        // process each mesh located at the current node
        for (u32 idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            LoadAssimpMesh(DirectoryPath, mesh, Scene, MeshData);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (u32 idx = 0; idx < Node->mNumChildren; ++idx)
        {
            LoadAssimpNode(DirectoryPath, Node->mChildren[idx], Scene, MeshData);
        }
    }

    void LoadAssimpMesh(const FString& DirectoryPath, aiMesh* Mesh, const aiScene* Scene,
                        FMeshMainMemoryBuffers& MeshData)
    {
        uint32_t currentTotalElementCount = MeshData.VertexCount;

        MeshData.VertexCount += Mesh->mNumVertices;
        for (uint32_t i = 0; i < Mesh->mNumVertices; ++i)
        {
            glm::vec3* VertexDataPointer = (glm::vec3*)(MeshData.VertexBuffer.Pointer + MeshData.VertexBuffer.Size);

            // Position
            VertexDataPointer->x = Mesh->mVertices[i].x;
            VertexDataPointer->y = Mesh->mVertices[i].y;
            VertexDataPointer->z = Mesh->mVertices[i].z;
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

            ElementPtr[0] = (currentTotalElementCount + Face->mIndices[0]);
            ElementPtr[1] = (currentTotalElementCount + Face->mIndices[1]);
            ElementPtr[2] = (currentTotalElementCount + Face->mIndices[2]);

            MeshData.ElementCount += 3;
            MeshData.ElementBuffer.Size += FaceDataSize;
        }
    }

    static CTextureResource* AssimpImportMaterialTexture(const FString& DirectoryPath, aiMaterial* Material, aiTextureType TextureType, const FString& MeshName, const FString& TextureTypeName)
    {
        // Import the texture
        aiString TextureFileName;
        Material->GetTexture(TextureType, 0, &TextureFileName);

        FDString TexturePath = SPrintf("%s/%s", *DirectoryPath, TextureFileName.C_Str());
        
        if (TexturesHolder.Contains(*TexturePath))
        {
            return TexturesHolder.Get(*TexturePath);
        }

        CTextureResource* Texture = ImportJPGTexture(TexturePath, true, gpu::ETextureDataType::UNSIGNED_BYTE, true, false, SPrintf("%s_%s", *MeshName, *TextureTypeName));

        if (Texture == nullptr)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to load %s texture of mesh %s", TextureTypeName, MeshName)
            Texture = TexturesHolder.GetDefaultResource();
        }

        TexturePath.Free();

        // Save to a texture asset

        FDString    TextureFilePath = SPrintf("assets/textures/%s_Texture%s.asset", *MeshName, *TextureTypeName);
        FILE*       TextureFile = fopen(*TextureFilePath, "wb");
        if (TextureFile == nullptr)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to save save imported %d texture of mesh %s - failed to open the file", *TextureTypeName, *MeshName);
            return nullptr;
        }

        Texture->SaveSynchronously(TextureFile);
        fclose(TextureFile);
        TextureFilePath.Free();
        
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

} // namespace lucid::resources