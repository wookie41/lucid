#include "resources/texture.hpp"
#include "stb_image.h"

#include <cassert>

namespace lucid::resources
{
    static bool texturesInitialized;

    ResourcesHolder<TextureResource> TexturesHolder;

    TextureResource* LoadTextureSTB(const String& TexturePath, const bool& IsTransparent)
    {
        int desiredChannels = IsTransparent ? 4 : 3;
        uint32_t channels;
        uint32_t Width, Height;

        stbi_uc* textureData = stbi_load(TexturePath, (int*)&Width, (int*)&Height, (int*)&channels, desiredChannels);
        assert(channels == desiredChannels);

        return new TextureResource{ (void*)textureData, Width, Height, false,
                                    IsTransparent ? TextureFormat::RGBA : TextureFormat::RGB };
    }

    void InitTextues()
    {
        if (texturesInitialized)
        {
            return;
        }

        texturesInitialized = true;

        TexturesHolder.SetDefaultResource(LoadTextureSTB("awesomeface.jpg", true));
    }

    TextureResource* LoadJPEG(char const* Path) { return LoadTextureSTB(Path, false); }
    TextureResource* LoadPNG(char const* Path) { return LoadTextureSTB(Path, true); }

    TextureResource::TextureResource(void* TextureData,
                                     const uint32_t& Width,
                                     const uint32_t& Height,
                                     const bool& IsGammeCorrected,
                                     const TextureFormat& Format)
    : textureData(TextureData), Width(Width), Height(Height), IsGammaCorrected(IsGammaCorrected),
      Format(Format)
    {
    }

    void TextureResource::FreeResource() { free(textureData); }
} // namespace lucid::resources