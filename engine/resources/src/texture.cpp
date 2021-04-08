#include "devices/gpu/texture.hpp"
#include "resources/texture.hpp"
#include "stb_image.h"
#include "common/log.hpp"
#include "platform/util.hpp"

#include <cassert>

namespace lucid::resources
{
    static bool texturesInitialized;

    CResourcesHolder<CTextureResource> TexturesHolder{};

    assets::FTextureAsset LoadTextureSTB(const FANSIString& InTexturePath,
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
        int NumDesiredChannels = InIsTransparent ? 4 : 3;
        uint32_t NumChannels;
        uint32_t Width, Height;

        stbi_set_flip_vertically_on_load(InFlipY);

        stbi_uc* TextureData = stbi_load(*InTexturePath, (int*)&Width, (int*)&Height, (int*)&NumChannels, NumDesiredChannels);
        assert(NumChannels == NumDesiredChannels);

        // gpu::CTexture* textureHandle = nullptr;
        // if (InSendToGPU)
        // {
        //     textureHandle = gpu::Create2DTexture(textureData, Width, Height, InDataType, InDataFormat, InPixelFormat, 0,
        //     InName); assert(textureHandle);
        // }

        LUCID_LOG(ELogLevel::INFO, "Loading texture %s took %f", *InTexturePath, platform::GetCurrentTimeSeconds() - start);

        assets::FTextureAsset TextureAsset;
        TextureAsset.Name = *InName;
        TextureAsset.Type = assets::EAssetType::TEXTURE;
        TextureAsset.Width = Width;
        TextureAsset.Height = Height;
        TextureAsset.DataType = InDataType;
        TextureAsset.DataFormat = InDataFormat;
        TextureAsset.PixelFormat = InPixelFormat;
        TextureAsset.bSRGB = true;
        TextureAsset.Data.Pointer = (char*)TextureData;
        TextureAsset.Data.Size = Width * Height * NumChannels * GetSizeInBytes(InDataType);
        return TextureAsset;
    }

    void InitTextures()
    {
        if (texturesInitialized)
        {
            return;
        }

        texturesInitialized = true;

        // TexturesHolder.SetDefaultResource(LoadTextureSTB(FString{ LUCID_TEXT("assets/textures/awesomeface.png") },
        //                                                  true,
        //                                                  gpu::ETextureDataType::UNSIGNED_BYTE,
        //                                                  gpu::ETextureDataFormat::SRGBA,
        //                                                  gpu::ETexturePixelFormat::RGBA,
        //                                                  true,
        //                                                  true,
        //                                                  FString{ "DefaultTexture" }));
    }

    CTextureResource* LoadTexture(const assets::FTextureAsset& Asset)
    {
        gpu::CTexture* TextureHandle = gpu::Create2DTexture(Asset.Data.Pointer,
                                                            Asset.Width,
                                                            Asset.Height,
                                                            Asset.DataType,
                                                            Asset.DataFormat,
                                                            Asset.PixelFormat,
                                                            0,
                                                            CopyToString(Asset.Name.c_str(), Asset.Name.length()));
        assert(TextureHandle);
        return new CTextureResource(Asset.Data.Pointer, TextureHandle, Asset.Width, Asset.Height, Asset.DataFormat, Asset.PixelFormat);
    }

    assets::FTextureAsset ImportJPGTexture(const FANSIString& InPath,
                                            const bool& InPerformGammaCorrection,
                                            const gpu::ETextureDataType& InDataType,
                                            const bool& InFlipY,
                                            const bool& InSendToGPU,
                                            const FANSIString& InName)
    {
        return LoadTextureSTB(InPath,
                       false,
                       InDataType,
                       InPerformGammaCorrection ? gpu::ETextureDataFormat::SRGB : gpu::ETextureDataFormat::RGB,
                       gpu::ETexturePixelFormat::RGB,
                       InFlipY,
                       InSendToGPU,
                       InName);
    }

    assets::FTextureAsset ImportPNGTexture(const FANSIString& InPath,
                              const bool& InPerformGammaCorrection,
                              const gpu::ETextureDataType& InDataType,
                              const bool& InFlipY,
                              const bool& InSendToGPU,
                              const FANSIString& InName)
    {
        return LoadTextureSTB(InPath,
                              true,
                              InDataType,
                              InPerformGammaCorrection ? gpu::ETextureDataFormat::SRGBA : gpu::ETextureDataFormat::RGBA,
                              gpu::ETexturePixelFormat::RGBA,
                              InFlipY,
                              InSendToGPU,
                              InName);
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