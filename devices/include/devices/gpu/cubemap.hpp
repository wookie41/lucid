#pragma once

#include "devices/gpu/texture.hpp"

namespace lucid::gpu
{
    class Cubemap : public Texture
    {

      public:
        enum class Face : u8
        {
            RIGHT,
            LEFT,
            UP,
            DOWN,
            BACK,
            FRONT
        };

        virtual void AttachAsColor(const u8& Index, Face InFace) = 0;

        virtual ~Cubemap() = default;
    };

    Cubemap* CreateCubemap(const glm::ivec2& Size,
                           TextureDataFormat InDataFormat,
                           TexturePixelFormat InPixelFormat,
                           TextureDataType DataType,
                           const char* FacesData[6] = nullptr);
} // namespace lucid::gpu