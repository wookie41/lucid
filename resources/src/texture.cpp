#include "resources/texture.hpp"
#include "stb_image.h"
#include "common/log.hpp"
#include "platform/util.hpp"

#include <cassert>

namespace lucid::resources
{
    static bool texturesInitialized;

    ResourcesHolder<TextureResource> TexturesHolder{};

    TextureResource* LoadTextureSTB(const String& TexturePath,
                                    const bool& IsTransparent,
                                    const bool& PerformGammaCorrection,
                                    const gpu::TextureDataType& DataType,
                                    const bool& FlipY,
                                    const bool& SendToGPU)
    {
#ifndef NDEBUG
        real start = platform::GetCurrentTimeSeconds();
#endif
        int desiredChannels = IsTransparent ? 4 : 3;
        uint32_t channels;
        uint32_t Width, Height;

        stbi_set_flip_vertically_on_load(FlipY);

        stbi_uc* textureData = stbi_load(*TexturePath, (int*)&Width, (int*)&Height, (int*)&channels, desiredChannels);
        assert(channels == desiredChannels);

        gpu::Texture* textureHandle = nullptr;
        if (SendToGPU)
        {
            textureHandle =
              gpu::Create2DTexture(textureData, Width, Height, DataType,
                                   IsTransparent ? gpu::TextureFormat::RGBA : gpu::TextureFormat::RGB, 0, PerformGammaCorrection);
            assert(textureHandle);
        }

        LUCID_LOG(LogLevel::INFO, "Loading texture %s took %f", *TexturePath,
                  platform::GetCurrentTimeSeconds() - start);

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

        TexturesHolder.SetDefaultResource(
          LoadTextureSTB("assets/textures/awesomeface.png", true, true, gpu::TextureDataType::UNSIGNED_BYTE, true, true));
    }

    TextureResource* LoadJPEG(const String& Path,
                              const bool& PerformGammaCorrection,
                              const gpu::TextureDataType& DataType,
                              const bool& FlipY,
                              const bool& SendToGPU)
    {
        return LoadTextureSTB(Path, false, PerformGammaCorrection, DataType, FlipY, SendToGPU);
    }
    TextureResource* LoadPNG(const String& Path,
                             const bool& PerformGammaCorrection,
                             const gpu::TextureDataType& DataType,
                             const bool& FlipY,
                             const bool& SendToGPU)
    {
        return LoadTextureSTB(Path, true, PerformGammaCorrection, DataType, FlipY, SendToGPU);
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