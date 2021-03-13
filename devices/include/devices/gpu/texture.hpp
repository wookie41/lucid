#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    enum class TextureDataType : u8
    {
        UNSIGNED_BYTE,
        FLOAT
    };

    enum class TextureType : u8
    {
        ONE_DIMENSIONAL,
        TWO_DIMENSIONAL,
        THREE_DIMENSIONAL
    };

    enum class MinTextureFilter : u8
    {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEARST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR
    };

    enum class MagTextureFilter : u8
    {
        NEAREST,
        LINEAR
    };

    enum class WrapTextureFilter : u8
    {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
    };

    enum class TextureDataFormat : u8
    {
        R,
        R16F,
        R32F,
        RG,
        RG16F,
        RG32F,
        RGB,
        RGB16F,
        RGB32F,
        RGBA,
        RGBA16F,
        RGBA32F,
        SRGB,
        SRGBA,
        DEPTH_COMPONENT,
        DEPTH_STENCIL
    };

    enum class TexturePixelFormat : u8
    {
        R,
        RG,
        RGB,
        RGBA,
        DEPTH_COMPONENT,
        DEPTH_STENCIL
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
                             const u32& Width,
                             const u32& Height,
                             const TextureDataType& DataType,
                             const TextureDataFormat& InDataFormat,
                             const TexturePixelFormat& InPixelFormat,
                             const int32_t& MipMapLevel);

    Texture* CreateEmpty2DTexture(const u32& Width, const u32& Height, const TextureDataType& DataType, const TextureDataFormat& InDataFormat, const TexturePixelFormat& InPixelFormat, const int32_t& MipMapLevel);
} // namespace lucid::gpu