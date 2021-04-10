#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/texture_enums.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class CTexture : public IFramebufferAttachment, public CGPUObject
    {
      public:
        CTexture(const FString& InName,
                 const ETextureDataType InTextureDataType,
                 const ETexturePixelFormat InTexturePixelFormat)
        : CGPUObject(InName), TextureDataType(InTextureDataType), TexturePixelFormat(InTexturePixelFormat)
        {
        }

        virtual void Bind() = 0;
        virtual glm::ivec3 GetDimensions() const = 0;

        virtual void SetMinFilter(const MinTextureFilter& Filter) = 0;
        virtual void SetMagFilter(const MagTextureFilter& Filter) = 0;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) = 0;
        virtual void SetWrapRFilter(const WrapTextureFilter& Filter) = 0;

        virtual ETextureDataType GetAttachmentDataType() const override { return TextureDataType; }

        virtual ETexturePixelFormat GetAttachmentPixelFormat() const override { return TexturePixelFormat; };

        virtual u64 GetSizeInBytes() const = 0;

        virtual void CopyPixels(void* DestBuffer, const u8& MipLevel) = 0;

        virtual ~CTexture() = default;

      protected:
        const ETextureDataType TextureDataType;
        const ETexturePixelFormat TexturePixelFormat;
    };

    CTexture* Create2DTexture(void* Data,
                              const uint32_t& Width,
                              const uint32_t& Height,
                              const ETextureDataType& InDataType,
                              const ETextureDataFormat& InDataFormat,
                              const ETexturePixelFormat& InPixelFormat,
                              const int32_t& MipMapLevel,
                              const FString& InName);

    CTexture* CreateEmpty2DTexture(const uint32_t& Width,
                                   const uint32_t& Height,
                                   const ETextureDataType& InDataType,
                                   const ETextureDataFormat& InDataFormat,
                                   const ETexturePixelFormat& InPixelFormat,
                                   const int32_t& MipMapLevel,
                                   const FString& InName);

    u8 GetSizeInBytes(const gpu::ETextureDataType& InType);
    u8 GetNumChannels(const gpu::ETexturePixelFormat& InType);
} // namespace lucid::gpu