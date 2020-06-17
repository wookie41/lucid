#pragma once

#include "devices/gpu/textures.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLTexture : public Texture
    {
      public:
        GLTexture(const GLuint& TextureID, const TextureType& Type, const math::ivec3& Size);

        virtual math::ivec3 GetSize() const override;
        virtual void Bind() override;
        virtual ~GLTexture();

      private:
        math::ivec3 size;
        GLenum textureType;
        GLuint glTextureHandle;
    };
} // namespace lucid::gpu