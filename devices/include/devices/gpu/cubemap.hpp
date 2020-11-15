#pragma once

#include "devices/gpu/texture.hpp"

namespace lucid::gpu
{
    class Cubemap : public Texture
    {

      public:
        enum class Face : uint8_t
        {
            RIGHT,
            LEFT,
            UP,
            DOWN,
            BACK,
            FRONT
        };

        virtual ~Cubemap() = default;
    };

    Cubemap* CreateCubemap(const glm::ivec2& Size,
                           TextureFormat InternalFormat,
                           TextureFormat DataFormat,
                           TextureDataType DataType,
                           const char* FacesData[6] = { nullptr });
} // namespace lucid::gpu