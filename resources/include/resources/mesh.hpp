#pragma once

#include "resources/holder.hpp"
#include "common/bytes.hpp"

#include "devices/gpu/vao.hpp"
#include <cstdint>

namespace lucid::resources
{
    class TextureResource;

    enum class MeshFeatures : u32
    {
        UV = 0x1,
        NORMALS = 0x2,
        TANGENTS = 0x4
    };

    // VertexData is only guaranteed to have positions
    // to find out whether the mesh contains things like
    // normals, uvs or bones data, check the MeshFeaturesFlags

    class MeshResource : public Resource
    {
      public:
        MeshResource(const u32& MeshFeaturesFlags,
                     TextureResource* MeshDiffuseMap,
                     TextureResource* MeshSpecularMap,
                     TextureResource* MeshNormalMap,
                     gpu::VertexArray* const MeshVAO,
                     gpu::Buffer* const MeshVertexBuffer,
                     gpu::Buffer* const MeshElementBuffer,
                     const MemBuffer& MeshVertexData,
                     const MemBuffer& MeshElementData);

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        const u32 FeaturesFlag;

        TextureResource* const DiffuseMap;
        TextureResource* const SpecularMap;
        TextureResource* const NormalMap;

        gpu::VertexArray* const VAO;
        gpu::Buffer* const VertexBuffer;
        gpu::Buffer* const ElementBuffer;

        const MemBuffer VertexData;
        const MemBuffer ElementData;
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

    MeshResource* AssimpLoadMesh(const String& DirectoryPath, const String& MeshFileName);

    extern ResourcesHolder<MeshResource> MeshesHolder;
} // namespace lucid::resources
