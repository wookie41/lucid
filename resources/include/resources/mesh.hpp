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

    // VertexData is only guaranteed to have positions
    // to find out whether the mesh contains things like
    // normals, uvs or bones data, check the MeshFeaturesFlags

    class CMeshResource : public CResource
    {
      public:
        CMeshResource(const u32& MeshFeaturesFlags,
                     CTextureResource* MeshDiffuseMap,
                     CTextureResource* MeshSpecularMap,
                     CTextureResource* MeshNormalMap,
                     gpu::CVertexArray* const MeshVAO,
                     gpu::CBuffer* const MeshVertexBuffer,
                     gpu::CBuffer* const MeshElementBuffer,
                     const FMemBuffer& MeshVertexData,
                     const FMemBuffer& MeshElementData);

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        const u32 FeaturesFlag;

        CTextureResource* const DiffuseMap;
        CTextureResource* const SpecularMap;
        CTextureResource* const NormalMap;

        gpu::CVertexArray* const VAO;
        gpu::CBuffer* const VertexBuffer;
        gpu::CBuffer* const ElementBuffer;

        const FMemBuffer VertexData;
        const FMemBuffer ElementData;
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

    CMeshResource* AssimpLoadMesh(gpu::FGPUState* InGPUState, const FANSIString& DirectoryPath, const FANSIString& MeshFileName);

    extern CResourcesHolder<CMeshResource> MeshesHolder;
} // namespace lucid::resources
