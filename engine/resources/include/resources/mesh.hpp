#pragma once

#include "resources/holder.hpp"
#include "common/bytes.hpp"

#include "devices/gpu/vao.hpp"
#include <cstdint>

namespace lucid::resources
{
    class CTextureResource;

    enum class EMeshFeatures : u32
    {
        UV = 0x1,
        NORMALS = 0x2,
        TANGENTS = 0x4
    };

    class CMeshResource : public CResource
    {
      public:
        CMeshResource(const UUID& InID,
                         const FString& InName,
                         const FString& InFilePath,
                         const u64& InOffset,
                         const u64& InDataSize);

        virtual EResourceType GetType() const override { return MESH; };

        virtual void LoadMetadata(FILE* ResourceFile) override;
        virtual void LoadDataToMainMemorySynchronously() override;
        virtual void LoadDataToVideoMemorySynchronously() override;

        virtual void SaveSynchronously(FILE* ResourceFile) override;
        
        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        u32 FeaturesFlag;

        gpu::CVertexArray*    VAO;
        gpu::CBuffer*         VertexBuffer;
        gpu::CBuffer*         ElementBuffer;

        FMemBuffer            VertexData;
        FMemBuffer            ElementData;
    };

    // Loads the mesh from the given directory, assumes to following things:
    // - 'DirectoryPath' ends with '/'
    // - diffuse map is stored in file named 'diffuseMap'
    // - specular map is stored in file named 'specularMap'
    // - normal map is stored in file named 'normalMap'

    // The loader doesn't support sub-mesh hierarchies, it flattens them and
    // loads in a single EBO and VBO it also requries the textures to be packed
    // by type, by that I mean
    // - single diffuse texture atlas
    // - single specular texture atlas
    // - single normal texture atlas
    // Diffuse and specular maps should be stored in JPEG, wheras normal maps
    // should be stored in PNG

    CMeshResource* AssimpLoadMesh(const FString& DirectoryPath, const FString& MeshFileName);

    extern CResourcesHolder<CMeshResource> MeshesHolder;
} // namespace lucid::resources
