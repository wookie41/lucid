#pragma once

#include "devices/gpu/texture.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLTexture : public Texture
    {
      public:
        GLTexture(const GLuint& TextureID, const TextureType& Type, const math::ivec3& TextureDimensions);

        virtual math::ivec3 GetDimensions() const override;

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void SetMinFilter(const MinTextureFilter& Filter) override;
        virtual void SetMagFilter(const MagTextureFilter& Filter) override;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) override;

        virtual ~GLTexture();

      private:
        math::ivec3 dimensions;
        GLenum textureType;
        GLuint glTextureHandle;
    };
} // namespace lucid::gpu