#pragma once

#include <cstdint>
#include "resources/holder.hpp"

namespace lucid::resources
{
    void InitTextures();

    enum class TextureFormat : uint8_t
    {
        RGB,
        RGBA
    };

    class TextureResource : public IResource
    {
      public:
        TextureResource(void* TextureData,
                        const uint32_t& Width,
                        const uint32_t& Height,
                        const bool& IsGammeCorrected,
                        const TextureFormat& Format);

        virtual void FreeResource() override;

        const uint32_t Width, Height;
        const bool IsGammaCorrected;
        const TextureFormat Format;

        inline const void* GetTextureData() const { return textureData; };

      private:
        void* textureData;
    };

    TextureResource* LoadJPEG(char const* Path);
    TextureResource* LoadPNG(char const* Path);

    extern ResourcesHolder<TextureResource> TexturesHolder;
} // namespace lucid::resources