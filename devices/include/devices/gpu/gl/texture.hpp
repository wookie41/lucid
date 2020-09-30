#pragma once

#include "devices/gpu/texture.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLTexture : public Texture
    {
      public:
        GLTexture(const GLuint& TextureID, const TextureType& Type, const math::ivec3& TextureDimensions);

        // Texture methods //

        virtual math::ivec3 GetDimensions() const override;

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void SetMinFilter(const MinTextureFilter& Filter) override;
        virtual void SetMagFilter(const MagTextureFilter& Filter) override;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) override;

        ///////////////////////////

        // Framebuffer attachment methods //

        virtual void AttachAsColor(const uint8_t& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        ///////////////////////////

        virtual ~GLTexture();

      private:
        bool isBound = false;
        math::ivec3 dimensions;
        GLenum textureType;
        GLuint glTextureHandle;
    };
} // namespace lucid::gpu