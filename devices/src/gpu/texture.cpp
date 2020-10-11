#define STB_IMAGE_IMPLEMENTATION

#include "devices/gpu/texture.hpp"
#include "stb_image.h"

#include <cassert>

#ifndef NDEBUG
#include <stdio.h>
#endif

namespace lucid::gpu
{
    void InitTextures() { stbi_set_flip_vertically_on_load(true); }

    static Texture* LoadWithSTBAndCreate(char const* Path, bool IsTransparent)
    {
        int desiredChannels = IsTransparent ? 4 : 3;
        int channels;
        glm::ivec2 size;

        stbi_uc* textureData = stbi_load(Path, &size.x, &size.y, &channels, desiredChannels);
        assert(channels == desiredChannels);

        Texture* createdTexture = Create2DTexture(textureData, size, 0, IsTransparent);

        stbi_image_free(textureData);
        return createdTexture;
    }

    Texture* CreateTextureFromJPEG(char const* Path) { return LoadWithSTBAndCreate(Path, false); }

    Texture* CreateTextureFromPNG(char const* Path) { return LoadWithSTBAndCreate(Path, true); }
} // namespace lucid::gpu
