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
                        const uint32_t& W,
                        const uint32_t& H,
                        const bool& GammeCorrected,
                        const gpu::TextureFormat& Fmt);

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        void* const TextureData;
        gpu::Texture* const TextureHandle;
        const uint32_t Width, Height;
        const bool IsGammaCorrected;
        const gpu::TextureFormat Format;
    };

    TextureResource* LoadJPEG(char const* Path);
    TextureResource* LoadPNG(char const* Path);

    extern ResourcesHolder<TextureResource> TexturesHolder;
} // namespace lucid::resources