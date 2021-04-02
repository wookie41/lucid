#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    enum class ETextureDataType : u8
    {
        UNSIGNED_BYTE,
        FLOAT,
        UNSIGNED_INT
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

    enum class ETextureDataFormat : u8
    {
        R,
        R16F,
        R32F,
        R32UI,
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

    enum class ETexturePixelFormat : u8
    {
        R,
        R_INTEGER,
        RG,
        RGB,
        RGBA,
        DEPTH_COMPONENT,
        DEPTH_STENCIL
    };
    
    class CTexture : public CFramebufferAttachment, public CGPUObject
    {
      public:

        explicit CTexture(const FANSIString& InName) : CGPUObject(InName) {}
        
        virtual void Bind() = 0;
        virtual glm::ivec3 GetDimensions() const = 0;

        virtual void SetMinFilter(const MinTextureFilter& Filter) = 0;
        virtual void SetMagFilter(const MagTextureFilter& Filter) = 0;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapRFilter(const WrapTextureFilter& Filter) = 0;

        virtual ~CTexture() = default;
    };

    CTexture* Create2DTexture(void* Data,
                              const uint32_t& Width,
                              const uint32_t& Height,
                              const ETextureDataType& DataType,
                              const ETextureDataFormat& InDataFormat,
                              const ETexturePixelFormat& InPixelFormat,
                              const int32_t& MipMapLevel,
                              const FANSIString& InName);

    CTexture* CreateEmpty2DTexture(const uint32_t& Width,
                                   const uint32_t& Height,
                                   const ETextureDataType& DataType,
                                   const ETextureDataFormat& InDataFormat,
                                   const ETexturePixelFormat& InPixelFormat,
                                   const int32_t& MipMapLevel,
                                   const FANSIString& InName);

} // namespace lucid::gpu