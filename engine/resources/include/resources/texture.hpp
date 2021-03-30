#pragma once

#include "devices/gpu/gpu.hpp"
#include "resources/holder.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid::resources
{
    void InitTextures();

    class CTextureResource : public CResource
    {
      public:
        CTextureResource(void* Data,
                        gpu::CTexture* Handle,
                        const u32& W,
                        const u32& H,
                        gpu::ETextureDataFormat InDataFormat,
                        gpu::ETexturePixelFormat InPixelFormat);

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        void* const TextureData;
        gpu::CTexture* const TextureHandle;
        const u32 Width, Height;
        gpu::ETextureDataFormat DataFormat;
        gpu::ETexturePixelFormat PixelFormat;
    };

    CTextureResource* LoadJPEG(const FANSIString& InPath,
                               const bool& InPerformGammaCorrection,
                               const gpu::ETextureDataType& InDataType,
                               const bool& InFlipY,
                               const bool& InSendToGPU,
                               const FANSIString& InName);

    CTextureResource* LoadPNG(const FANSIString& InPath,
                              const bool& InPerformGammaCorrection,
                              const gpu::ETextureDataType& InDataType,
                              const bool& InFlipY,
                              const bool& InSendToGPU
                              ,const FANSIString& InName);

    extern CResourcesHolder<CTextureResource> TexturesHolder;
} // namespace lucid::resources
