#pragma once
#include "common/types.hpp"

namespace lucid::gpu
{
    enum class ETextureDataType : u8
    {
        UNSIGNED_BYTE,
        FLOAT,
        UNSIGNED_INT
    };

    enum class TextureType : u8
    {
        ONE_DIMENSIONAL,
        TWO_DIMENSIONAL,
        THREE_DIMENSIONAL
    };

    enum class MinTextureFilter : u8
    {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEARST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR
    };

    enum class MagTextureFilter : u8
    {
        NEAREST,
        LINEAR
    };

    enum class WrapTextureFilter : u8
    {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
    };

    enum class ETextureDataFormat : u8
    {
        R,
        R16F,
        R32F,
        R32UI,
        RG,
        RG16F,
        RG32F,
        RGB,
        RGB16F,
        RGB32F,
        RGBA,
        RGBA16F,
        RGBA32F,
        SRGB,
        SRGBA,
        DEPTH_COMPONENT,
        DEPTH_STENCIL
    };

    enum class ETexturePixelFormat : u8
    {
        RED,
        RED_INTEGER,
        RG,
        RGB,
        RGBA,
        DEPTH_COMPONENT,
        DEPTH_STENCIL
    };
} // namespace lucid::gpu
