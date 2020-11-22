#include "resources/mesh.hpp"

#include "common/log.hpp"
#include "common/bytes.hpp"

#include "glm/glm.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "resources/texture.hpp"
#include "devices/gpu/buffer.hpp"

namespace lucid::resources
{

    MeshResource::MeshResource(const uint32_t& MeshFeaturesFlags,
                               TextureResource* MeshDiffuseMap,
                               TextureResource* MeshSpecularMap,
                               TextureResource* MeshNormalMap,
                               gpu::VertexArray* const MeshVAO,
                               gpu::Buffer* const MeshVertexBuffer,
                               gpu::Buffer* const MeshElementBuffer,
                               const MemBuffer& MeshVertexData,
                               const MemBuffer& MeshElementData)
    : FeaturesFlag(MeshFeaturesFlags), DiffuseMap(MeshDiffuseMap), SpecularMap(MeshSpecularMap), NormalMap(MeshNormalMap),
      VAO(MeshVAO), VertexBuffer(MeshVertexBuffer), ElementBuffer(MeshElementBuffer), VertexData(MeshVertexData),
      ElementData(MeshElementData)
    {
    }
    void MeshResource::FreeMainMemory()
    {
        if (!isMainMemoryFreed)
        {
            isMainMemoryFreed = true;
            free(VertexData.Pointer);
            free(ElementData.Pointer);
        }
    }

    void MeshResource::FreeVideoMemory()
    {
        if (!isVideoMemoryFreed)
        {
            isVideoMemoryFreed = true;
            VertexBuffer->Free();
            ElementBuffer->Free();
            VAO->Free();
        }
    }

    ResourcesHolder<MeshResource> MeshesHolder;

    static Assimp::Importer assimpImporter;

    static const constexpr uint32_t ASSIMP_DEFAULT_FLAGS = aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                           aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                                                           aiProcess_OptimizeMeshes;

    struct MeshSize
    {
        uint32_t VertexDataSize = 0;
        uint32_t ElementDataSize = 0;
    };

    struct MeshCPUData
    {
        MemBuffer VertexBuffer;
        MemBuffer ElementBuffer;
        uint32_t VertexCount = 0;
        uint32_t ElementCount = 0;
    };

    struct MeshGPUData
    {
        gpu::VertexArray* VAO = nullptr;
        gpu::Buffer* VertexBuffer = nullptr;
        gpu::Buffer* ElementBuffer = nullptr;
    };

    static MeshSize calculateMeshDataSize(aiNode* Node, const aiScene* Scene);

    static void loadAssimpNode(const String& DirectoryPath, aiNode* Node, const aiScene* Scene, MeshCPUData& meshData);

    static void loadAssimpMesh(const String& DirectoryPath, aiMesh* mesh, const aiScene* scene, MeshCPUData& meshData);

    static uint32_t determineMeshFeatures(const aiScene* Root);

    MeshGPUData sendMeshToGPU(const uint32_t& Features, const MeshCPUData& MeshData);

    static TextureResource*
    loadMaterialTexture(char const* DirectoryPath, aiMaterial* Material, aiTextureType TextureType, bool IsPNGFormat);

    MeshResource* AssimpLoadMesh(const String& DirectoryPath, const String& MeshFileName)
    {
        // read mesh file

        DString meshFilePath = Concat(DirectoryPath, MeshFileName);

        const aiScene* root = assimpImporter.ReadFile(meshFilePath, ASSIMP_DEFAULT_FLAGS);

        if (!root || root->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !root->mRootNode)
        {
            LUCID_LOG(LogLevel::WARN, "Assimp failed to load model %s", assimpImporter.GetErrorString())
            meshFilePath.Free();
            return nullptr;
        }

        // allocate main memory for the mesh

        MeshSize meshDataSize = calculateMeshDataSize(root->mRootNode, root);

        MemBuffer vertexDataBuffer = CreateMemBuffer(meshDataSize.VertexDataSize);
        MemBuffer elementDataBuffer =
          meshDataSize.ElementDataSize > 0 ? CreateMemBuffer(meshDataSize.ElementDataSize) : MemBuffer{ nullptr, 0, 0 };

        MeshCPUData meshData{ vertexDataBuffer, elementDataBuffer, 0, 0 };

        // load the assimp node //

        loadAssimpNode(DirectoryPath, root->mRootNode, root, meshData);

        LUCID_LOG(LogLevel::INFO, "Vertex data  %u/%u, element data %u/%u", meshData.VertexBuffer.Length,
                  meshData.VertexBuffer.Capacity, meshData.ElementBuffer.Length, meshData.ElementBuffer.Capacity);

        uint32_t meshFeatures = determineMeshFeatures(root);

        // load the material //

        aiString textureFilePath;
        aiMaterial* material = root->mMaterials[1]; // we require the texture atlases to be stored in the root node

        TextureResource* diffuseMap = nullptr;
        TextureResource* specularMap = nullptr;
        TextureResource* normalMap = nullptr;

        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            diffuseMap = loadMaterialTexture(DirectoryPath, material, aiTextureType_DIFFUSE, false);
        }

        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            specularMap = loadMaterialTexture(DirectoryPath, material, aiTextureType_SPECULAR, false);
        }

        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            normalMap = loadMaterialTexture(DirectoryPath, material, aiTextureType_HEIGHT, false);
        }

        meshFilePath.Free();

        // send the data to the gpu

        MeshGPUData meshGPUData = sendMeshToGPU(meshFeatures, meshData);

        return new MeshResource{ meshFeatures,
                                 diffuseMap,
                                 specularMap,
                                 normalMap,
                                 meshGPUData.VAO,
                                 meshGPUData.VertexBuffer,
                                 meshGPUData.ElementBuffer,
                                 meshData.VertexBuffer,
                                 meshData.ElementBuffer };
    }; // namespace lucid::resources

    static MeshSize calculateMeshDataSize(aiNode* Node, const aiScene* Scene)
    {
        MeshSize meshSize;
        for (uint32_t idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            uint32_t vertexSize = 0;

            vertexSize += sizeof(glm::vec3); // position

            // @Caution this might break when there is some weird mesh 
            // that doesnt have all of the vertex properties on all of the vertices
            // this should really happend, but it's worth to have it mind and 
            // just make it safer in the future, when when have tools that
            // convert the meshes to engine's internal representation

            if (mesh->HasNormals())
            {
                vertexSize += sizeof(glm::vec3);
            }

            if (mesh->HasTangentsAndBitangents())
            {
                vertexSize += sizeof(glm::vec3);
            }

            if (mesh->HasTangentsAndBitangents())
            {
                vertexSize += sizeof(glm::vec2);
            }

            meshSize.VertexDataSize += mesh->mNumVertices * vertexSize;
            meshSize.ElementDataSize += (mesh->mNumFaces * 3 * sizeof(uint32_t));
        }

        // recursively calculate size of the children
        for (unsigned int idx = 0; idx < Node->mNumChildren; ++idx)
        {
            MeshSize subMeshSize = calculateMeshDataSize(Node->mChildren[idx], Scene);
            meshSize.VertexDataSize += subMeshSize.VertexDataSize;
            meshSize.ElementDataSize += subMeshSize.ElementDataSize;
        }

        return meshSize;
    }

    static uint32_t determineMeshFeatures(const aiScene* Root)
    {
        uint32_t meshFeatures = 0;

        if (Root->mMeshes[0]->HasTextureCoords(0))
        {
            meshFeatures |= static_cast<uint32_t>(MeshFeatures::UV);
        }

        if (Root->mMeshes[0]->HasNormals())
        {
            meshFeatures |= static_cast<uint32_t>(MeshFeatures::NORMALS);
        }

        if (Root->mMeshes[0]->HasTangentsAndBitangents())
        {
            meshFeatures |= static_cast<uint32_t>(MeshFeatures::TANGENTS);
        }

        return meshFeatures;
    }

    static void loadAssimpNode(const String& DirectoryPath, aiNode* Node, const aiScene* Scene, MeshCPUData& meshData)
    {
        // process each mesh located at the current node
        for (uint32_t idx = 0; idx < Node->mNumMeshes; ++idx)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            loadAssimpMesh(DirectoryPath, mesh, Scene, meshData);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (uint32_t idx = 0; idx < Node->mNumChildren; ++idx)
        {
            loadAssimpNode(DirectoryPath, Node->mChildren[idx], Scene, meshData);
        }
    }

    void loadAssimpMesh(const String& DirectoryPath, aiMesh* Mesh, const aiScene* Scene, MeshCPUData& MeshData)
    {
        uint32_t currentTotalElementCount = MeshData.VertexCount;

        MeshData.VertexCount += Mesh->mNumVertices;
        for (uint32_t i = 0; i < Mesh->mNumVertices; ++i)
        {

            glm::vec3* vertexPtr = (glm::vec3*)(MeshData.VertexBuffer.Pointer + MeshData.VertexBuffer.Length);
            // positions
            vertexPtr->x = Mesh->mVertices[i].x;
            vertexPtr->y = Mesh->mVertices[i].y;
            vertexPtr->z = Mesh->mVertices[i].z;
            vertexPtr += 1;

            MeshData.VertexBuffer.Length += sizeof(glm::vec3);

            if (Mesh->HasNormals())
            {
                // load normals
                vertexPtr->x = Mesh->mNormals[i].x;
                vertexPtr->y = Mesh->mNormals[i].y;
                vertexPtr->z = Mesh->mNormals[i].z;
                vertexPtr += 1;
                MeshData.VertexBuffer.Length += sizeof(glm::vec3);
            }

            if (Mesh->HasTangentsAndBitangents())
            {
                // load tangents
                vertexPtr->x = Mesh->mTangents[i].x;
                vertexPtr->y = Mesh->mTangents[i].y;
                vertexPtr->z = Mesh->mTangents[i].z;

                vertexPtr += 1;
                MeshData.VertexBuffer.Length += sizeof(glm::vec3);
            }

            if (Mesh->HasTextureCoords(0))
            {
                glm::vec2* texPtr = (glm::vec2*)vertexPtr;

                texPtr->x = Mesh->mTextureCoords[0][i].x;
                texPtr->y = Mesh->mTextureCoords[0][i].y;
                vertexPtr = (glm::vec3*)(texPtr + 1);
                MeshData.VertexBuffer.Length += sizeof(glm::vec2);
            }
        }

        // now wak through each of the mesh's faces (a face in a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int idx = 0; idx < Mesh->mNumFaces; ++idx)
        {
            aiFace* face = Mesh->mFaces + idx;
            // copy the face's indicies to the element buffer
            uint32_t* elementPtr = (uint32_t*)(MeshData.ElementBuffer.Pointer + MeshData.ElementBuffer.Length);
            const uint32_t faceDataSize = 3 * sizeof(uint32_t);

            elementPtr[0] = (currentTotalElementCount + face->mIndices[0]);
            elementPtr[1] = (currentTotalElementCount + face->mIndices[1]);
            elementPtr[2] = (currentTotalElementCount + face->mIndices[2]);

            MeshData.ElementCount += 3;
            MeshData.ElementBuffer.Length += faceDataSize;
        }
    }

    static TextureResource*
    loadMaterialTexture(char const* DirectoryPath, aiMaterial* Material, aiTextureType TextureType, bool IsPNGFormat)
    {
        aiString textureFileName;
        Material->GetTexture(TextureType, 0, &textureFileName);
        DString texturePath = Concat(DirectoryPath, textureFileName.C_Str());
        if (TexturesHolder.Contains(texturePath))
        {
            return TexturesHolder.Get((char*)texturePath);
        }

        auto textureMap = IsPNGFormat ? LoadPNG((char*)texturePath, true, gpu::TextureDataType::UNSIGNED_BYTE, true) : LoadJPEG((char*)texturePath, true, gpu::TextureDataType::UNSIGNED_BYTE, true);
        if (textureMap == nullptr)
        {
            textureMap = TexturesHolder.GetDefaultResource();
        }
        else
        {
            TexturesHolder.Add((char*)texturePath, textureMap);
        }

        texturePath.Free();

        return textureMap;
    }

    MeshGPUData sendMeshToGPU(const uint32_t& Features, const MeshCPUData& MeshData)
    {
        gpu::BufferDescription bufferDescription;

        // sending vertex data to the gpu
        bufferDescription.data = MeshData.VertexBuffer.Pointer;
        bufferDescription.size = MeshData.VertexBuffer.Length;

        gpu::Buffer* gpuVertexBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);
        gpu::Buffer* gpuElementBuffer = nullptr;

        // sending element to the gpu if it's present
        if (MeshData.ElementBuffer.Pointer)
        {
            bufferDescription.data = MeshData.ElementBuffer.Pointer;
            bufferDescription.size = MeshData.ElementBuffer.Length;

            gpuElementBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);
        }

        // prepare vertex array attributes

        // position is always present
        uint8_t numOfAttribues = 1;
        uint8_t stride = sizeof(glm::vec3);

        uint32_t currentOffset = sizeof(glm::vec3);
        uint32_t normalsOffset = 0;
        uint32_t tangentsOffset = 0;
        uint32_t uvOffset = 0;

        if (Features & static_cast<uint32_t>(MeshFeatures::NORMALS))
        {
            numOfAttribues += 1;
            stride += sizeof(glm::vec3);
            normalsOffset = currentOffset;
            currentOffset += sizeof(glm::vec3);
        }

        if (Features & static_cast<uint32_t>(MeshFeatures::TANGENTS))
        {
            numOfAttribues += 1;
            stride += sizeof(glm::vec3);
            tangentsOffset = currentOffset;
            currentOffset += sizeof(glm::vec3);
        }

        if (Features & static_cast<uint32_t>(MeshFeatures::UV))
        {
            numOfAttribues += 1;
            stride += sizeof(glm::vec2);
            uvOffset = currentOffset;
            currentOffset += sizeof(glm::vec2);
        }

        StaticArray<gpu::VertexAttribute> attributes(numOfAttribues);
        attributes.Add({ 0, 3, Type::FLOAT, false, stride, 0, 0 });

        uint32_t attributeIdx = 1;
        if (Features & static_cast<uint32_t>(MeshFeatures::NORMALS))
            attributes.Add({ attributeIdx++, 3, Type::FLOAT, false, stride, normalsOffset, 0 });

        if (Features & static_cast<uint32_t>(MeshFeatures::TANGENTS))
            attributes.Add({ attributeIdx++, 3, Type::FLOAT, false, stride, tangentsOffset, 0 });

        if (Features & static_cast<uint32_t>(MeshFeatures::UV))
            attributes.Add({ attributeIdx++, 2, Type::FLOAT, false, stride, uvOffset, 0 });

        gpu::VertexArray* vao = gpu::CreateVertexArray(&attributes, gpuVertexBuffer, gpuElementBuffer, gpu::DrawMode::TRIANGLES,
                                                       MeshData.VertexCount, MeshData.ElementCount);
        attributes.Free();

        return MeshGPUData{ vao, gpuVertexBuffer, gpuElementBuffer };
    }

} // namespace lucid::resources