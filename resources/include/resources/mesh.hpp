#pragma once

#include "resources/holder.hpp"
#include "common/bytes.hpp"

#include <cstdint>

namespace lucid::resources
{
    class TextureResource;

    enum class MeshFeatures : uint32_t
    {
        UV = 0x1,
        NORMALS = 0x2,
        TANGENTS = 0x4
    };

    // VertexData is only guaranteed to have positions
    // to find out whether the mesh contains things like
    // normals, uvs or bones data, check the MeshFeaturesFlags

    class MeshResource : public IResource
    {
      public:
        MeshResource(const uint32_t& FeaturesFlags,
                     TextureResource* DiffuseMap,
                     TextureResource* SpecularMap,
                     TextureResource* NormalMap,
                     const MemBuffer& VertexData,
                     const MemBuffer& ElementData);

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        const uint32_t MeshFeaturesFlag;

        TextureResource* const DiffuseMap;
        TextureResource* const SpecularMap;
        TextureResource* const NormalMap;
        const MemBuffer vertexData;
        const MemBuffer elementData;
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
