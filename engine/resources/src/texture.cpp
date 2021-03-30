#include "resources/texture.hpp"
#include "stb_image.h"
#include "common/log.hpp"
#include "platform/util.hpp"

#include <cassert>

namespace lucid::resources
{
    static bool texturesInitialized;

    CResourcesHolder<CTextureResource> TexturesHolder{};

    CTextureResource* LoadTextureSTB(const FANSIString& InTexturePath,
                                    const bool& InIsTransparent,
                                    const gpu::ETextureDataType& InDataType,
                                    gpu::ETextureDataFormat InDataFormat,
                                    gpu::ETexturePixelFormat InPixelFormat,
                                    const bool& InFlipY,
                                    const bool& InSendToGPU,
                                    const FANSIString& InName)
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

        gpu::CTexture* textureHandle = nullptr;
        if (InSendToGPU)
        {
            textureHandle = gpu::Create2DTexture(textureData, Width, Height, InDataType, InDataFormat, InPixelFormat, 0, InName);
            assert(textureHandle);
        }

        LUCID_LOG(ELogLevel::INFO, "Loading texture %s took %f", *InTexturePath, platform::GetCurrentTimeSeconds() - start);

        return new CTextureResource{ (void*)textureData, textureHandle, Width, Height, InDataFormat, InPixelFormat };
    }

    void InitTextures()
    {
        if (texturesInitialized)
        {
            return;
        }

        texturesInitialized = true;

        TexturesHolder.SetDefaultResource(LoadTextureSTB(FString{ LUCID_TEXT("assets/textures/awesomeface.png") }, true,
                                                         gpu::ETextureDataType::UNSIGNED_BYTE, gpu::ETextureDataFormat::SRGBA,
                                                         gpu::ETexturePixelFormat::RGBA, true, true, FString {"DefaultTexture"}));
    }

    CTextureResource* LoadJPEG(const FANSIString& InPath,
                              const bool& InPerformGammaCorrection,
                              const gpu::ETextureDataType& InDataType,
                              const bool& InFlipY,
                              const bool& InSendToGPU,
                              const FANSIString& InName)
    {
        return LoadTextureSTB(InPath, false, InDataType, InPerformGammaCorrection ? gpu::ETextureDataFormat::SRGB : gpu::ETextureDataFormat::RGB, gpu::ETexturePixelFormat::RGB,  InFlipY, InSendToGPU, InName);
    }
    CTextureResource* LoadPNG(const FANSIString& InPath,
                              const bool& InPerformGammaCorrection,
                              const gpu::ETextureDataType& InDataType,
                              const bool& InFlipY,
                              const bool& InSendToGPU,
                              const FANSIString& InName)
    {
        return LoadTextureSTB(InPath, true, InDataType,
                              InPerformGammaCorrection ? gpu::ETextureDataFormat::SRGBA : gpu::ETextureDataFormat::RGBA,
                              gpu::ETexturePixelFormat::RGBA, InFlipY, InSendToGPU, InName);
    }

    CTextureResource::CTextureResource(void* Data,
                                     gpu::CTexture* Handle,
                                     const uint32_t& W,
                                     const uint32_t& H,
                                     gpu::ETextureDataFormat InDataFormat,
                                     gpu::ETexturePixelFormat InPixelFormat)
    : TextureData(Data), TextureHandle(Handle), Width(W), Height(H), DataFormat(InDataFormat), PixelFormat(InPixelFormat)
    {
    }

    void CTextureResource::FreeMainMemory()
    {
        if (!IsMainMemoryFreed)
        {
            IsMainMemoryFreed = true;
            free(TextureData);
        }
    }
    void CTextureResource::FreeVideoMemory()
    {
        if (!IsVideoMemoryFreed)
        {
            TextureHandle->Free();
            IsVideoMemoryFreed = true;
        }
    }
} // namespace lucid::resources