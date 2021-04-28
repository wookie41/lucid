#include "resources/texture_resource.hpp"

#include "stb_image.h"
#include "common/log.hpp"
#include "platform/util.hpp"
#include "devices/gpu/texture.hpp"
#include "resources/serialization_versions.hpp"

#include <cassert>

namespace lucid::resources
{
#define TEXTURE_RESOURCE_METADATA_SIZE                                                                          \
    (sizeof(u8) + sizeof(u32) + sizeof(u32) + sizeof(gpu::ETextureDataFormat) + sizeof(gpu::ETextureDataType) + \
     sizeof(gpu::ETexturePixelFormat))

    static bool texturesInitialized;

    CTextureResource* LoadTextureSTB(const FString& InTexturePath,
                                     const FString& InResourcePath,
                                     const bool& InIsTransparent,
                                     const gpu::ETextureDataType& InDataType,
                                     gpu::ETextureDataFormat InDataFormat,
                                     gpu::ETexturePixelFormat InPixelFormat,
                                     const bool& InFlipY,
                                     const bool& InSendToGPU,
                                     const FString& InName)
    {
#ifndef NDEBUG
        real StartTime = platform::GetCurrentTimeSeconds();
#endif
        int NumDesiredChannels = InIsTransparent ? 4 : 3;

        u32 NumChannels;
        u32 Width, Height;

        stbi_set_flip_vertically_on_load(InFlipY);

        stbi_uc* TextureData = stbi_load(*InTexturePath, (int*)&Width, (int*)&Height, (int*)&NumChannels, NumDesiredChannels);
        u64 TextureSize = Width * Height * NumChannels * GetSizeInBytes(InDataType);

        assert(NumChannels == NumDesiredChannels);

        auto* TextureResource =
          new CTextureResource(sole::uuid4(), InName, InResourcePath, 0, TextureSize, TEXTURE_SERIALIZATION_VERSION);

        TextureResource->bSRGB = true;
        TextureResource->DataType = InDataType;
        TextureResource->DataFormat = InDataFormat;
        TextureResource->PixelFormat = InPixelFormat;
        TextureResource->TextureData = TextureData;
        TextureResource->Width = Width;
        TextureResource->Height = Height;

        if (InSendToGPU)
        {
            TextureResource->LoadDataToVideoMemorySynchronously();
        }

        LUCID_LOG(ELogLevel::INFO, "Loading texture %s took %f", *InTexturePath, platform::GetCurrentTimeSeconds() - StartTime);

        return TextureResource;
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

    CTextureResource* ImportJPGTexture(const FString& InPath,
                                       const FString& InResourcePath,
                                       const bool& InPerformGammaCorrection,
                                       const gpu::ETextureDataType& InDataType,
                                       const bool& InFlipY,
                                       const bool& InSendToGPU,
                                       const FString& InName)
    {
        return LoadTextureSTB(InPath,
                              InResourcePath,
                              false,
                              InDataType,
                              InPerformGammaCorrection ? gpu::ETextureDataFormat::SRGB : gpu::ETextureDataFormat::RGB,
                              gpu::ETexturePixelFormat::RGB,
                              InFlipY,
                              InSendToGPU,
                              InName);
    }

    CTextureResource* ImportPNGTexture(const FString& InPath,
                                       const FString& InResourcePath,
                                       const bool& InPerformGammaCorrection,
                                       const gpu::ETextureDataType& InDataType,
                                       const bool& InFlipY,
                                       const bool& InSendToGPU,
                                       const FString& InName)
    {
        return LoadTextureSTB(InPath,
                              InResourcePath,
                              true,
                              InDataType,
                              InPerformGammaCorrection ? gpu::ETextureDataFormat::SRGBA : gpu::ETextureDataFormat::RGBA,
                              gpu::ETexturePixelFormat::RGBA,
                              InFlipY,
                              InSendToGPU,
                              InName);
    }

    CTextureResource::CTextureResource(const UUID& InID,
                                       const FString& InName,
                                       const FString& InFilePath,
                                       const u64& InOffset,
                                       const u64& InDataSize,
                                       const u32& InAssetSerializationVersion)
    : CResource(InID, InName, InFilePath, InOffset, InDataSize, InAssetSerializationVersion)
    {
    }

    void CTextureResource::LoadMetadata(FILE* ResourceFile)
    {
        fread_s(&bSRGB, sizeof(bSRGB), sizeof(bSRGB), 1, ResourceFile);
        fread_s(&Width, sizeof(Width), sizeof(Width), 1, ResourceFile);
        fread_s(&Height, sizeof(Height), sizeof(Height), 1, ResourceFile);
        fread_s(&DataFormat, sizeof(DataFormat), sizeof(DataFormat), 1, ResourceFile);
        fread_s(&DataType, sizeof(DataType), sizeof(DataType), 1, ResourceFile);
        fread_s(&PixelFormat, sizeof(PixelFormat), sizeof(PixelFormat), 1, ResourceFile);
    }

    void CTextureResource::LoadDataToMainMemorySynchronously()
    {
        // Check if we already loaded it
        if (TextureData)
        {
            return;
        }

        FILE* TextureFile;
        if (fopen_s(&TextureFile, *FilePath, "rb") != 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to open file %s", *FilePath);
            return;
        }

        fseek(TextureFile, Offset + RESOURCE_FILE_HEADER_SIZE + Name.GetLength() + TEXTURE_RESOURCE_METADATA_SIZE, SEEK_SET);

        TextureData = malloc(DataSize);

        const u64 ReadBytes = fread_s(TextureData, DataSize, 1, DataSize, TextureFile);
        assert(ReadBytes == DataSize);
        fclose(TextureFile);
    }

    void CTextureResource::LoadDataToVideoMemorySynchronously()
    {
        // For now we require for the data to be loaded in the main memory
        assert(TextureData);

        // Check if we didn't already load it
        if (TextureHandle)
        {
            return;
        }

        TextureHandle = gpu::Create2DTexture(TextureData, Width, Height, DataType, DataFormat, PixelFormat, 0, Name);
        assert(TextureHandle);
    }

    void CTextureResource::SaveSynchronously(FILE* ResourceFile) const
    {
        assert(TextureData);

        // Write header
        SaveHeader(ResourceFile);

        // Write metadata
        fwrite(&bSRGB, sizeof(bSRGB), 1, ResourceFile);
        fwrite(&Width, sizeof(Width), 1, ResourceFile);
        fwrite(&Height, sizeof(Height), 1, ResourceFile);
        fwrite(&DataFormat, sizeof(DataFormat), 1, ResourceFile);
        fwrite(&DataType, sizeof(DataType), 1, ResourceFile);
        fwrite(&PixelFormat, sizeof(PixelFormat), 1, ResourceFile);

        // Write texture data
        fwrite(TextureData, DataSize, 1, ResourceFile);
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

    CTextureResource* LoadTexture(const FString& FilePath)
    {
        FILE* TextureFile;
        if (fopen_s(&TextureFile, *FilePath, "rb") != 0)
        {
            LUCID_LOG(ELogLevel::ERR, "Failed to load texture from file %s", *FilePath);
            return nullptr;
        }

        CTextureResource* TextureResource = resources::LoadResource<resources::CTextureResource>(TextureFile, FilePath);
        fclose(TextureFile);
        return TextureResource;
    }

    void CTextureResource::MigrateToLatestVersion() { Resave(); }
} // namespace lucid::resources