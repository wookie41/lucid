#pragma once

#include "common/strings.hpp"
#include "resources/resource.hpp"

namespace lucid::gpu
{
    class CTexture;
    enum class ETextureDataType :u8;
    enum class ETextureDataFormat :u8;
    enum class ETexturePixelFormat :u8;
}

namespace lucid::resources
{
    void InitTextures();

    class CTextureResource : public CResource
    {
      public:
        CTextureResource(const UUID& InUUID,
                         const FString& InName,
                         const FString& InFilePath,
                         const u64& InOffset,
                         const u64& InDataSize);

        virtual EResourceType GetType() const override { return TEXTURE; };

        virtual void LoadMetadata(FILE* ResourceFile) override;
        virtual void LoadDataToMainMemorySynchronously() override;
        virtual void LoadDataToVideoMemorySynchronously() override;

        virtual void SaveSynchronously(FILE* ResourceFile = nullptr) override;

        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        void* TextureData = nullptr;
        u8 bSRGB = 0;
        gpu::CTexture* TextureHandle = nullptr;

        u32 Width = 0;
        u32 Height = 0;

        gpu::ETextureDataType DataType;
        gpu::ETextureDataFormat DataFormat;
        gpu::ETexturePixelFormat PixelFormat;
    };

    CTextureResource* LoadTexture(const FString& FilePath);

    CTextureResource* ImportJPGTexture(const FString& InPath,
                                       const bool& InPerformGammaCorrection,
                                       const gpu::ETextureDataType& InDataType,
                                       const bool& InFlipY,
                                       const bool& InSendToGPU,
                                       const FString& InName);

    CTextureResource* ImportPNGTexture(const FString& InPath,
                                       const bool& InPerformGammaCorrection,

                                       const gpu::ETextureDataType& InDataType,
                                       const bool& InFlipY,
                                       const bool& InSendToGPU,
                                       const FString& InName);
} // namespace lucid::resources
