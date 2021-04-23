#pragma once

#include "devices/gpu/texture.hpp"
#include "GL/glew.h"

#if DEVELOPMENT
#include "imgui.h"
#endif

namespace lucid::gpu
{
    class CGLTexture : public CTexture
    {
      public:
        CGLTexture(const GLuint& InGLTextureID,
                  const GLenum& InGLTextureTarget,
                  const u32& InWidth,
                  const u32& InHeight,
                  const FString& InName,
                  const GLenum& InGLPixelFormat,
                  const GLenum& InGLTextureDataType,
                  const u64& InSizeInBytes,
                  const ETextureDataType& InDataType,
                  const ETexturePixelFormat& InPixelFormat);

    public:

        /** Texture methods */
        virtual void SetObjectName() override;

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

#if DEVELOPMENT
        virtual void ImGuiDrawToImage(const ImVec2& InImageSize) const override;
        virtual bool ImGuiImageButton(const ImVec2& InImageSize) const override;
#endif  
        
        ///////////////////////////

        virtual void Free() override;
        virtual ~CGLTexture() = default;


      private:

        const GLenum    GLTextureTarget;
        const GLuint    GLTextureHandle;
        const GLenum    GLPixelFormat;
        const GLenum    GLTextureDataType;
        const u64       SizeInBytes;
    };
} // namespace lucid::gpu