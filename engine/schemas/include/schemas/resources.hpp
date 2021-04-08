#include "common/bytes.hpp"
#include "devices/gpu/texture_enums.hpp"

ENUM_BEGIN(lucid::assets, EAssetType, "The type of assets the engine supports")
    ENUM_ITEM(TEXTURE, "")
ENUM_END()

STRUCT_BEGIN(lucid::assets, FTextureAsset, "A texture asset that can be imported to the engine")
    STRUCT_FIELD(std::string, Name, "", "Name under which it'll be available in the engine")
    STRUCT_FIELD(lucid::assets::EAssetType, Type, EAssetType::TEXTURE, "Type of the asset")
    STRUCT_FIELD(u32, Width, 0, "Width of the texture")
    STRUCT_FIELD(u32, Height, 0, "Height of the texture")
    STRUCT_FIELD(bool, bSRGB, false, "Wether to the texture data is in sRGB color space or linear")
    STRUCT_FIELD(lucid::gpu::ETextureDataType, DataType, lucid::gpu::ETextureDataType::UNSIGNED_BYTE, "Type of the texture data")
    STRUCT_FIELD(lucid::gpu::ETextureDataFormat, DataFormat, lucid::gpu::ETextureDataFormat::RG, "Format of the texture data")
    STRUCT_FIELD(lucid::gpu::ETexturePixelFormat, PixelFormat, lucid::gpu::ETexturePixelFormat::RG, "Format of the texture pixels")
    STRUCT_FIELD(FBinaryData, Data, lucid::EMPTY_BINARY_DATA, "Texture data")
STRUCT_END()