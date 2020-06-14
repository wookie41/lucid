#define STB_IMAGE_IMPLEMENTATION

#include "devices/gpu/textures.hpp"
#include "stb_image.h"

#include <cassert>

#ifndef NDEBUG
#include <stdio.h>
#endif


namespace lucid::gpu
{
    Texture* CreateTextureFromJPEG(const char const* Path)
    {
        int channels;
        math::ivec2 size;

        stbi_uc* textureData = stbi_load(Path, &size.x, &size.y, &channels, 3);
        assert(channels == 3);

#ifndef NDEBUG
        printf("Loadad a JPEG of size %dx%d\n", size.x ,size.y);
#endif

        Texture* createdTexture = Create2DTexture(textureData, size, 0, false);

        stbi_image_free(textureData);
        return createdTexture;
    }
} // namespace lucid::gpu
