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

    CTextureResource* ImportTexture(const FString& InPath,
                                    const FString& InResourcePath,
                                    const bool& InPerformGammaCorrection,
                                    const gpu::ETextureDataType& InDataType,
                                    const bool& InFlipY,
                                    const bool& InSendToGPU,
                                    const FString& InName)
    {
#ifndef NDEBUG
        real StartTime = platform::GetCurrentTimeSeconds();
#endif
        u32 NumChannels;
        u32 Width, Height;

        stbi_set_flip_vertically_on_load(InFlipY);

        stbi_uc* TextureData = stbi_load(*InPath, (int*)&Width, (int*)&Height, (int*)&NumChannels, 0);
        u64 TextureSize = Width * Height * NumChannels * GetSizeInBytes(InDataType);

        auto* TextureResource =
          new CTextureResource(sole::uuid4(), InName, InResourcePath, 0, TextureSize, TEXTURE_SERIALIZATION_VERSION);

        TextureResource->bSRGB = InPerformGammaCorrection;
        TextureResource->DataType = InDataType;
        if (InPerformGammaCorrection)
        {
            switch (NumChannels)
            {
            case 3:
                TextureResource->DataFormat = gpu::ETextureDataFormat::SRGB;
                break;
            case 4:
                TextureResource->DataFormat = gpu::ETextureDataFormat::SRGBA;
                break;
            default:
                assert(0);
            }
        }
        else
        {
            switch (NumChannels)
            {
            case 1:
                TextureResource->DataFormat = gpu::ETextureDataFormat::R;
                break;
            case 2:
                TextureResource->DataFormat = gpu::ETextureDataFormat::RG;
                break;
            case 3:
                TextureResource->DataFormat = gpu::ETextureDataFormat::RGB;
                break;
            case 4:
                TextureResource->DataFormat = gpu::ETextureDataFormat::RGBA;
                break;
            default:
                assert(0);
            }
        }

        switch (NumChannels)
        {
        case 1:
            TextureResource->PixelFormat = gpu::ETexturePixelFormat::RED;
            break;
        case 2:
            TextureResource->PixelFormat = gpu::ETexturePixelFormat::RG;
            break;
        case 3:
            TextureResource->PixelFormat = gpu::ETexturePixelFormat::RGB;
            break;
        case 4:
            TextureResource->PixelFormat = gpu::ETexturePixelFormat::RGBA;
            break;
        default:
            assert(0);
        }

        TextureResource->TextureData = TextureData;
        TextureResource->Width = Width;
        TextureResource->Height = Height;

        if (InSendToGPU)
        {
            TextureResource->LoadDataToVideoMemorySynchronously();
        }

        LUCID_LOG(ELogLevel::INFO, "Loading texture %s took %f", *InPath, platform::GetCurrentTimeSeconds() - StartTime);

        return TextureResource;
    }

    void InitTextures()
    {
        if (texturesInitialized)
        {
            return;
        }

        texturesInitialized = true;
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