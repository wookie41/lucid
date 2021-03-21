#pragma once

#include "devices/gpu/texture.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLTexture : public CTexture
    {
      public:
        GLTexture(const GLuint& TextureID, const TextureType& Type, const glm::ivec3& TextureDimensions);
        GLTexture(const GLuint& TextureID, const glm::ivec3& Dimensions, const GLenum& TextureTaget);

        // Texture methods //

        virtual glm::ivec3 GetDimensions() const override;

        virtual glm::ivec2 GetSize() const override { return { dimensions.x, dimensions.y }; }

        virtual void Bind() override;

        virtual void SetMinFilter(const MinTextureFilter& Filter) override;
        virtual void SetMagFilter(const MagTextureFilter& Filter) override;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapRFilter(const WrapTextureFilter& Filter) override;

        ///////////////////////////

        // Framebuffer attachment methods //

        virtual void AttachAsColor(const u8& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        ///////////////////////////

        virtual void Free() override;
        virtual ~GLTexture() = default;

      private:
        glm::ivec3 dimensions;
        GLenum glTextureTarget;
        GLuint glTextureHandle;
    };
} // namespace lucid::gpu