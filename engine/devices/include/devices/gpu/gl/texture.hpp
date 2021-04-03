#pragma once

#include "devices/gpu/texture.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class CGLTexture : public CTexture
    {
      public:
        CGLTexture(const GLuint& InGLTextureID,
                  const GLenum& InGLTextureTarget,
                  const glm::ivec3& InTextureDimensions,
                  const FANSIString& InName,
                  const GLenum& InGLPixelFormat,
                  const GLenum& InGLTextureDataType,
                  const u64& InSizeInBytes,
                  const ETextureDataType& InDataType,
                  const ETexturePixelFormat& InPixelFormat);

    public:

        // Texture methods //

        virtual void SetObjectName() override;

        virtual glm::ivec3 GetDimensions() const override;

        virtual glm::ivec2 GetSize() const override { return { Dimensions.x, Dimensions.y }; }

        virtual void Bind() override;

        virtual void SetMinFilter(const MinTextureFilter& Filter) override;
        virtual void SetMagFilter(const MagTextureFilter& Filter) override;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapRFilter(const WrapTextureFilter& Filter) override;

        virtual u64 GetSizeInBytes() const override;
        
        virtual void CopyPixels(void* DestBuffer, const u8& MipLevel) override;
        
        ///////////////////////////

        // Framebuffer attachment methods //


        virtual void AttachAsColor(const u8& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        ///////////////////////////

        virtual void Free() override;
        virtual ~CGLTexture() = default;

      private:

        glm::ivec3 Dimensions;
        const GLenum GLTextureTarget;
        const GLuint GLTextureHandle;
        const GLenum GLPixelFormat;
        const GLenum GLTextureDataType;
        u64 SizeInBytes;
    };
} // namespace lucid::gpu