#include "resources/mesh.hpp"

#include "common/log.hpp"
#include "common/bytes.hpp"

#include "glm/glm.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "resources/texture.hpp"

namespace lucid::resources
{
    MeshResource::MeshResource(const uint32_t& FeaturesFlags,
                               TextureResource* DiffuseMap,
                               TextureResource* SpecularMap,
                               TextureResource* NormalMap,
                               const MemBuffer& VertexData,
                               const MemBuffer& ElementData)
    : MeshFeaturesFlag(FeaturesFlags), DiffuseMap(DiffuseMap), SpecularMap(SpecularMap), NormalMap(NormalMap), vertexData(VertexData), elementData(ElementData)
    {
    }

    void MeshResource::FreeResource()
    {
        free(vertexData.Pointer);
        free(elementData.Pointer);
    }

    static ResourcesHolder<MeshResource> MeshesHolder;

    static Assimp::Importer assimpImporter;

    static const constexpr uint32_t ASSIMP_DEFAULT_FLAGS =
    aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes;

    struct MeshSize
    {
        uint32_t VertexDataSize = 0;
        uint32_t ElementDataSize = 0;
    };

    static MeshSize calculateMeshDataSize(aiNode* Node, const aiScene* Scene);
    static void loadMeshData(const String& DirectoryPath, aiNode* Node, const aiScene* Scene, MemBuffer& VertexDataBuffer, MemBuffer& ElementDataBuffer);
    static void loadAssimpMesh(const String& DirectoryPath, aiMesh* mesh, const aiScene* scene, MemBuffer& VertexDataBuffer, MemBuffer& ElementDataBuffer);

    static TextureResource* loadMaterialTexture(char const* DirectoryPath, aiMaterial* Material, aiTextureType TextureType, bool IsPNGFormat);

    MeshResource* LoadMesh(const String& DirectoryPath, const String& MeshFileName)
    {
        DString meshFilePath = Concat(DirectoryPath, MeshFileName);

        const aiScene* scene = assimpImporter.ReadFile(meshFilePath, ASSIMP_DEFAULT_FLAGS);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LUCID_LOG(LogLevel::WARN, "Assimp failed to load model %s", assimpImporter.GetErrorString())
            meshFilePath.Free();
            return nullptr;
        }

        const MeshSize meshDataSize = calculateMeshDataSize(scene->mRootNode, scene);

        MemBuffer vertexDataBuffer = CreateMemBuffer(meshDataSize.VertexDataSize);
        MemBuffer elementDataBuffer = CreateMemBuffer(meshDataSize.ElementDataSize);

        loadMeshData(DirectoryPath, scene->mRootNode, scene, vertexDataBuffer, elementDataBuffer);

        LUCID_LOG(LogLevel::INFO, "Vertex data  %d/%d, element data %d/%d", vertexDataBuffer.Length, vertexDataBuffer.Capacity, vertexDataBuffer.Length, vertexDataBuffer.Capacity);

        // load the material
        aiString textureFilePath;
        aiMaterial* material = scene->mMaterials[0]; // we require the texture atlases to be stored in the root node

        TextureResource* diffuseMap = loadMaterialTexture(DirectoryPath, material, aiTextureType_DIFFUSE, false);
        TextureResource* specularMap = loadMaterialTexture(DirectoryPath, material, aiTextureType_SPECULAR, false);
        TextureResource* normalMap = loadMaterialTexture(DirectoryPath, material, aiTextureType_NORMALS, true);

        meshFilePath.Free();

        return new MeshResource{ MeshFeatures::UV, diffuseMap, specularMap, normalMap, vertexDataBuffer, elementDataBuffer };
    };

    static MeshSize calculateMeshDataSize(aiNode* Node, const aiScene* Scene)
    {
        MeshSize meshSize;
        for (uint32_t idx = 0; idx < Node->mNumMeshes; idx++)
        {
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            meshSize.VertexDataSize += mesh->mNumVertices * ((sizeof(float) * 3) + //  positions
                                                             (sizeof(float) * 3) + //  normals
                                                             (sizeof(float) * 3) + //  tangents
                                                             (sizeof(float) * 2) //  textureCoords
                                                            );
            meshSize.ElementDataSize += (mesh->mNumFaces * 3 * sizeof(int));
        }

        // recursively calculate size of the children
        for (unsigned int i = 0; i < Node->mNumChildren; i++)
        {
            MeshSize subMeshSize = calculateMeshDataSize(Node->mChildren[i], Scene);
            meshSize.VertexDataSize += subMeshSize.VertexDataSize;
            meshSize.ElementDataSize += subMeshSize.ElementDataSize;
        }

        return meshSize;
    }

    static void loadMeshData(const String& DirectoryPath, aiNode* Node, const aiScene* Scene, MemBuffer& VertexDataBuffer, MemBuffer& ElementDataBuffer)
    {
        // process each mesh located at the current node
        for (uint32_t idx = 0; idx < Node->mNumMeshes; idx++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = Scene->mMeshes[Node->mMeshes[idx]];
            loadAssimpMesh(DirectoryPath, mesh, Scene, VertexDataBuffer, ElementDataBuffer);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < Node->mNumChildren; i++)
        {
            loadMeshData(DirectoryPath, Node->mChildren[i], Scene, VertexDataBuffer, ElementDataBuffer);
        }
    }

    void loadAssimpMesh(const String& DirectoryPath, aiMesh* Mesh, const aiScene* Scene, MemBuffer& VertexDataBuffer, MemBuffer& ElementDataBuffer)
    {
        // Normal used when mesh doesn't have normals
        static glm::vec3 dummyNormal{ 0, 0, 1 };

        aiVector3D* vertexPtr = (aiVector3D*)(VertexDataBuffer.Pointer + VertexDataBuffer.Length);
        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < Mesh->mNumVertices; i++)
        {
            // positions
            *vertexPtr = Mesh->mVertices[i];
            vertexPtr += 1;
            VertexDataBuffer.Length += sizeof(aiVector3D);

            // normals
            *vertexPtr = Mesh->mNormals[i];
            vertexPtr += 1;
            VertexDataBuffer.Length += sizeof(aiVector3D);

            // tangents
            *vertexPtr = Mesh->mTangents[i];
            vertexPtr += 1;
            VertexDataBuffer.Length += sizeof(aiVector3D);

            aiVector2D* texPtr = (aiVector2D*)vertexPtr;
            if (Mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                (texPtr->x) = Mesh->mTextureCoords[0][i].x;
                (texPtr->y) = Mesh->mTextureCoords[0][i].y;
                VertexDataBuffer.Length += sizeof(aiVector2D);
            }
            else
            {
                *texPtr = { 0, 0 };
                VertexDataBuffer.Length += sizeof(aiVector2D);
            }
        }

        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < Mesh->mNumFaces; i++)
        {
            aiFace face = Mesh->mFaces[i];
            // copy the face's indicies to the element buffer
            uint32_t* elementPtr = (uint32_t*)(ElementDataBuffer.Pointer + ElementDataBuffer.Length);
            const uint32_t faceDataSize = 3 * sizeof(uint32_t) * face.mNumIndices;
            memcpy(elementPtr, face.mIndices, faceDataSize);
            ElementDataBuffer.Length += faceDataSize;
        }
    }

    static TextureResource* loadMaterialTexture(char const* DirectoryPath, aiMaterial* Material, aiTextureType TextureType, bool IsPNGFormat)
    {
        aiString textureFileName;
        Material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFileName);
        DString texturePath = Concat(DirectoryPath, textureFileName.C_Str());
        if (TexturesHolder.Contains(texturePath))
        {
            return TexturesHolder.Get((char*)texturePath);
        }

        auto textureMap = IsPNGFormat ? LoadPNG((char*)texturePath) : LoadJPEG((char*)texturePath);
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

} // namespace lucid::resources