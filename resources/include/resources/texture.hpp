#pragma once

#include <cstdint>
#include "resources/holder.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid::resources
{
    void InitTextures();

    class TextureResource : public Resource
    {
      public:
        TextureResource(void* Data,
                        gpu::Texture* Handle,
                        const u32& W,
                        const u32& H,
                        const bool& GammeCorrected,
                        const gpu::TextureFormat& Fmt);

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        void* const TextureData;
        gpu::Texture* const TextureHandle;
        const u32 Width, Height;
        const bool IsGammaCorrected;
        const gpu::TextureFormat Format;
    };

    TextureResource* LoadJPEG(const ANSIString& InPath,
                              const bool& InPerformGammaCorrection,
                              const gpu::TextureDataType& InDataType,
                              const bool& InFlipY = false,
                              const bool& InSendToGPU = true);
    TextureResource* LoadPNG(const ANSIString& InPath,
                             const bool& InPerformGammaCorrection,
                             const gpu::TextureDataType& InDataType,
                             const bool& InFlipY = false,
                             const bool& InSendToGPU = true);

    extern ResourcesHolder<TextureResource> TexturesHolder;
} // namespace lucid::resources