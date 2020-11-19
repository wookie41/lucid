#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    enum class TextureDataType : uint8_t
    {
        UNSIGNED_BYTE,
        FLOAT
    };

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

    enum class TextureFormat : uint8_t
    {
        RGB,
        RGBA,
        SRGB,
        SRGBA,
        RGB16F,
        DEPTH_COMPONENT
    };

    class Texture : public FramebufferAttachment
    {
      public:
        virtual void Bind() = 0;
        virtual glm::ivec3 GetDimensions() const = 0;

        virtual void SetMinFilter(const MinTextureFilter& Filter) = 0;
        virtual void SetMagFilter(const MagTextureFilter& Filter) = 0;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapRFilter(const WrapTextureFilter& Filter) = 0;

        virtual void Free() = 0;
        virtual ~Texture() = default;
    };

    Texture* Create2DTexture(void* Data,
                             const uint32_t& Width,
                             const uint32_t& Height,
                             const TextureDataType& DataType,
                             const TextureFormat& Format,
                             const int32_t& MipMapLevel,
                             const bool& PerformGammaCorrection);

    Texture*
    CreateEmpty2DTexture(const uint32_t& Width, const uint32_t& Height, const TextureDataType& DataType, const TextureFormat& Format, const int32_t& MipMapLevel);
} // namespace lucid::gpu