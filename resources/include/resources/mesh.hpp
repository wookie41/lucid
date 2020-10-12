#pragma once

namespace lucid::resources
{
    class TextureResource;
    struct MeshResource
    {

        void* VertexData;
        void* ElementData;

        TextureResource* DiffuseTextureResource; 
        TextureResource* DiffuseTextureResource; 
    };
} // namespace lucid::resources
