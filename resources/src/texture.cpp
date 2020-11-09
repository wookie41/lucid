#include "resources/texture.hpp"
#include "stb_image.h"

#include <cassert>

namespace lucid::resources
{
    static bool texturesInitialized;

    ResourcesHolder<TextureResource> TexturesHolder{};

    TextureResource* LoadTextureSTB(const String& TexturePath, const bool& IsTransparent, const bool& PerformGammaCorrection)
    {
        int desiredChannels = IsTransparent ? 4 : 3;
        uint32_t channels;
        uint32_t Width, Height;

        stbi_uc* textureData = stbi_load(TexturePath, (int*)&Width, (int*)&Height, (int*)&channels, desiredChannels);
        assert(channels == desiredChannels);

        gpu::Texture* textureHandle =
          gpu::Create2DTexture(textureData, Width, Height, IsTransparent ? gpu::TextureFormat::RGBA : gpu::TextureFormat::RGB, 0,
                               PerformGammaCorrection);
        assert(textureHandle);

        return new TextureResource{ (void*)textureData,
                                    textureHandle,
                                    Width,
                                    Height,
                                    false,
                                    IsTransparent ? gpu::TextureFormat::RGBA : gpu::TextureFormat::RGB };
    }

    void InitTextures()
    {
        if (texturesInitialized)
        {
            return;
        }

        texturesInitialized = true;

        TexturesHolder.SetDefaultResource(LoadTextureSTB("assets/textures/awesomeface.png", true, true));
    }

    TextureResource* LoadJPEG(char const* Path, const bool& PerformGammaCorrection)
    {
        return LoadTextureSTB(Path, false, PerformGammaCorrection);
    }
    TextureResource* LoadPNG(char const* Path, const bool& PerformGammaCorrection)
    {
        return LoadTextureSTB(Path, true, PerformGammaCorrection);
    }

    TextureResource::TextureResource(void* Data,
                                     gpu::Texture* Handle,
                                     const uint32_t& W,
                                     const uint32_t& H,
                                     const bool& GammeCorrected,
                                     const gpu::TextureFormat& Fmt)
    : TextureData(Data), TextureHandle(Handle), Width(W), Height(H), IsGammaCorrected(GammeCorrected), Format(Fmt)
    {
    }

    void TextureResource::FreeMainMemory()
    {
        if (!isMainMemoryFreed)
        {
            isMainMemoryFreed = true;
            free(TextureData);
        }
    }
    void TextureResource::FreeVideoMemory()
    {
        if (!isVideoMemoryFreed)
        {
            TextureHandle->Free();
            isVideoMemoryFreed = true;
        }
    }
} // namespace lucid::resources