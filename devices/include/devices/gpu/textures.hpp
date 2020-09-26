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

    enum class MinTextureFilter : uint8_t
    {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEARST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR
    };

    enum class MagTextureFilter : uint8_t
    {
        NEAREST,
        LINEAR
    };

    enum class WrapTextureFilter : uint8_t
    {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
    };

    // TODO Texture MIN/MAG filters, sampling methods
    class Texture
    {
      public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual math::ivec3 GetSize() const = 0;

        virtual void SetMinFilter(const MinTextureFilter& Filter) = 0;
        virtual void SetMagFilter(const MagTextureFilter& Filter) = 0;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) = 0;

        virtual ~Texture() = default;
    };

    Texture* CreateTextureFromJPEG(const char const* Path);
    Texture* CreateTextureFromPNG(const char const* Path);

    Texture* Create2DTexture(const void const* TextureData,
                             const math::ivec2& TextureSize,
                             const int32_t MipMapLevel,
                             bool IsTransparent);
} // namespace lucid::gpu