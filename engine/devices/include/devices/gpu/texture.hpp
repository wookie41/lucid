#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/texture_enums.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class CTexture : public IFramebufferAttachment, public CGPUObject
    {
      public:
        CTexture(const FString&             InName,
                 const u32&                 InWidth,
                 const u32&                 InHeight,
                 const ETextureDataType&    InTextureDataType,
                 const ETextureDataFormat&  InTextureDataFormat,
                 const ETexturePixelFormat& InTexturePixelFormat)
        : CGPUObject(InName), TextureDataType(InTextureDataType), TexturePixelFormat(InTexturePixelFormat), Width(InWidth), Height(InHeight),
          TextureDataFormat(InTextureDataFormat)
        {
        }

        virtual void Bind() = 0;

        inline u32 GetWidth() const { return Width; }
        inline u32 GetHeight() const { return Height; }

        virtual void SetMinFilter(const EMinTextureFilter& Filter)    = 0;
        virtual void SetMagFilter(const EMagTextureFilter& Filter)    = 0;
        virtual void SetWrapSFilter(const EWrapTextureFilter& Filter) = 0;
        virtual void SetWrapTFilter(const EWrapTextureFilter& Filter) = 0;
        virtual void SetWrapRFilter(const EWrapTextureFilter& Filter) = 0;
        virtual void SetBorderColor(const FColor& InColor)            = 0;

        virtual ETextureDataType    GetAttachmentDataType() const override { return TextureDataType; }
        virtual ETextureDataFormat  GetAttachmentDataFormat() const override { return TextureDataFormat; }
        virtual ETexturePixelFormat GetAttachmentPixelFormat() const override { return TexturePixelFormat; };

        virtual u64 GetSizeInBytes() const = 0;

        virtual void CopyPixels(void* DestBuffer, const u8& MipLevel) const = 0;

        /** Bindless texture stuff */
        virtual u64  GetBindlessHandle()               = 0;
        virtual bool IsBindlessTextureResident() const = 0;
        virtual void MakeBindlessResident()            = 0;
        virtual void MakeBindlessNonResident()         = 0;

        virtual ~CTexture() = default;

#if DEVELOPMENT
        virtual void ImGuiDrawToImage(const ImVec2& InImageSize) const = 0;
        virtual bool ImGuiImageButton(const ImVec2& InImageSize) const = 0;
#endif

      protected:
        const u32                 Width;
        const u32                 Height;
        const ETextureDataType    TextureDataType;
        const ETextureDataFormat  TextureDataFormat;
        const ETexturePixelFormat TexturePixelFormat;
    };

    CTexture* Create2DTexture(void*                      Data,
                              const uint32_t&            Width,
                              const uint32_t&            Height,
                              const ETextureDataType&    InDataType,
                              const ETextureDataFormat&  InDataFormat,
                              const ETexturePixelFormat& InPixelFormat,
                              const int32_t&             MipMapLevel,
                              const FString&             InName);

    CTexture* CreateEmpty2DTexture(const uint32_t&            Width,
                                   const uint32_t&            Height,
                                   const ETextureDataType&    InDataType,
                                   const ETextureDataFormat&  InDataFormat,
                                   const ETexturePixelFormat& InPixelFormat,
                                   const int32_t&             MipMapLevel,
                                   const FString&             InName);

    u8 GetSizeInBytes(const gpu::ETextureDataType& InType);
    u8 GetNumChannels(const gpu::ETexturePixelFormat& InType);
} // namespace lucid::gpu