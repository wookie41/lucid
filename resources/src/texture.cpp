#include "resources/texture.hpp"
#include "stb_image.h"
#include "common/log.hpp"
#include "platform/util.hpp"

#include <cassert>

namespace lucid::resources
{
    static bool texturesInitialized;

    ResourcesHolder<TextureResource> TexturesHolder{};

    TextureResource* LoadTextureSTB(const ANSIString& InTexturePath,
                                    const bool& InIsTransparent,
                                    const bool& InPerformGammaCorrection,
                                    const gpu::TextureDataType& InDataType,
                                    const bool& InFlipY,
                                    const bool& InSendToGPU)
    {
#ifndef NDEBUG
        real start = platform::GetCurrentTimeSeconds();
#endif
        int desiredChannels = InIsTransparent ? 4 : 3;
        uint32_t channels;
        uint32_t Width, Height;

        stbi_set_flip_vertically_on_load(InFlipY);

        stbi_uc* textureData = stbi_load(*InTexturePath, (int*)&Width, (int*)&Height, (int*)&channels, desiredChannels);
        assert(channels == desiredChannels);

        gpu::Texture* textureHandle = nullptr;
        if (InSendToGPU)
        {
            textureHandle =
              gpu::Create2DTexture(textureData, Width, Height, InDataType,
                                   InIsTransparent ? gpu::TextureFormat::RGBA : gpu::TextureFormat::RGB, 0, InPerformGammaCorrection);
            assert(textureHandle);
        }

        LUCID_LOG(LogLevel::INFO, "Loading texture %s took %f", *InTexturePath,
                  platform::GetCurrentTimeSeconds() - start);

        return new TextureResource{ (void*)textureData,
                                    textureHandle,
                                    Width,
                                    Height,
                                    false,
                                    InIsTransparent ? gpu::TextureFormat::RGBA : gpu::TextureFormat::RGB };
    }

    void InitTextures()
    {
        if (texturesInitialized)
        {
            return;
        }

        texturesInitialized = true;

        TexturesHolder.SetDefaultResource(LoadTextureSTB(String { LUCID_TEXT("assets/textures/awesomeface.png") }, true, true, gpu::TextureDataType::UNSIGNED_BYTE, true, true));
    }

    TextureResource* LoadJPEG(const ANSIString& InPath,
                              const bool& InPerformGammaCorrection,
                              const gpu::TextureDataType& InDataType,
                              const bool& InFlipY,
                              const bool& InSendToGPU)
    {
        return LoadTextureSTB(InPath, false, InPerformGammaCorrection, InDataType, InFlipY, InSendToGPU);
    }
    TextureResource* LoadPNG(const ANSIString& InPath,
                             const bool& InPerformGammaCorrection,
                             const gpu::TextureDataType& InDataType,
                             const bool& InFlipY,
                             const bool& InSendToGPU)
    {
        return LoadTextureSTB(InPath, true, InPerformGammaCorrection, InDataType, InFlipY, InSendToGPU);
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