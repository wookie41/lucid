#pragma once

#include "common/math.hpp"

namespace lucid::gpu
{
    void InitTextures();

    enum class TextureType : uint8_t
    {
        ONE_DIMENSIONAL,
        TWO_DIMENSIONAL,
        THREE_DIMENSIONAL
    };

    // TODO Texture MIN/MAG filters, sampling methods
    class Texture
    {
      public:
        virtual void Bind() = 0;
        virtual math::ivec3 GetSize() const = 0;
        virtual ~Texture() = default;
    };

    Texture* CreateTextureFromJPEG(const char const* Path);
    Texture* CreateTextureFromPNG(const char const* Path);

    Texture* Create2DTexture(const void const* TextureData,
                             const math::ivec2& TextureSize,
                             const int32_t MipMapLevel,
                             bool IsTransparent);
} // namespace lucid::gpu