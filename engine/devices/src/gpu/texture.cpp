#include "devices/gpu/texture.hpp"

#include <cassert>

#ifndef NDEBUG
#include <stdio.h>
#endif

namespace lucid::gpu
{
    u8 GetSizeInBytes(const gpu::ETextureDataType& InType)
    {
        switch (InType)
        {
        case ETextureDataType::UNSIGNED_BYTE:
            return 1;
        case ETextureDataType::FLOAT:
            return 4;
        case ETextureDataType::UNSIGNED_INT:
            return 4;
        default:
            assert(0);
        }
    }

    u8 GetNumChannels(const gpu::ETexturePixelFormat& InType)
    {
        switch (InType)
        {
        case ETexturePixelFormat::RED:
        case ETexturePixelFormat::RED_INTEGER:
        case ETexturePixelFormat::DEPTH_COMPONENT:
        case ETexturePixelFormat::DEPTH_STENCIL:
            return 1;
        case ETexturePixelFormat::RG:
            return 2;
        case ETexturePixelFormat::RGB:
            return 3;
        case ETexturePixelFormat::RGBA:
            return 4;
        default:
            assert(0);
        }
    }

} // namespace lucid::gpu
