﻿#include "devices/gpu/cubemap.hpp"

namespace lucid::gpu
{
    CCubemap::CCubemap(const FString& InName,
                       const u32& InWidth,
                       const u32& InHeight,
                       const ETextureDataType InTextureDataType,
                       const ETexturePixelFormat InTexturePixelFormat) : CTexture(InName, InWidth, InHeight, InTextureDataType, InTexturePixelFormat)
    {
    }
}