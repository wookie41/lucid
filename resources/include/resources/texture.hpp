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

    TextureResource* LoadJPEG(const String& Path,
                              const bool& PerformGammaCorrection,
                              const gpu::TextureDataType& DataType,
                              const bool& FlipY = false,
                              const bool& SendToGPU = true);
    TextureResource* LoadPNG(const String& Path,
                             const bool& PerformGammaCorrection,
                             const gpu::TextureDataType& DataType,
                             const bool& FlipY = false,
                             const bool& SendToGPU = true);

    extern ResourcesHolder<TextureResource> TexturesHolder;
} // namespace lucid::resources