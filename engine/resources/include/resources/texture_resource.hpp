#pragma once

#include <devices/gpu/texture.hpp>

#include "common/strings.hpp"
#include "resources/resource.hpp"

namespace lucid::gpu
{
    class CTexture;
    enum class ETextureDataType : u8;
    enum class ETextureDataFormat : u8;
    enum class ETexturePixelFormat : u8;
} // namespace lucid::gpu

namespace lucid::resources
{
    void InitTextures();

    class CTextureResource : public CResource
    {
      public:
        CTextureResource(const UUID&    InUUID,
                         const FString& InName,
                         const FString& InFilePath,
                         const u64&     InOffset,
                         const u64&     InDataSize,
                         const u32&     InAssetSerializationVersion);

        virtual EResourceType GetType() const override { return TEXTURE; };

        virtual void LoadMetadata(FILE* ResourceFile) override;
        virtual void LoadDataToMainMemorySynchronously() override;
        virtual void LoadDataToVideoMemorySynchronously() override;

        virtual void SaveSynchronously(FILE* ResourceFile = nullptr) const override;
        virtual void MigrateToLatestVersion() override;

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        virtual void LoadThumbnail() override;
        virtual void MakeThumbnail() override;

        void*          TextureData           = nullptr;
        u8             bSRGB                 = 0;
        gpu::CTexture* TextureHandle         = nullptr;
        
        u32            Width                 = 0;
        u32            Height                = 0;

        gpu::ETextureDataType    DataType;
        gpu::ETextureDataFormat  DataFormat;
        gpu::ETexturePixelFormat PixelFormat;
    };

    CTextureResource* LoadTexture(const FString& FilePath);

    CTextureResource* ImportTexture(const FString&               InPath,
                                    const FString&               InResourcePath,
                                    const bool&                  InPerformGammaCorrection,
                                    const gpu::ETextureDataType& InDataType,
                                    const bool&                  InFlipY,
                                    const bool&                  InSendToGPU,
                                    const FString&               InName);
} // namespace lucid::resources
