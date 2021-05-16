#pragma once

#include "devices/gpu/cubemap.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class CGLCubemap : public CCubemap
    {
      public:
        explicit CGLCubemap(const GLuint& Handle,
                            const u32& InWidth,
                            const u32& InHeight,
                            const FString& InName,
                            const ETextureDataType& InTextureDataType,
                            const ETextureDataFormat& InTextureDataFormat,
                            const ETexturePixelFormat& InTexturePixelFormat);

        virtual void SetObjectName() override;

        /** Framebuffer attachment interface */

        virtual void AttachAsColor(const u8& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        virtual u64 GetSizeInBytes() const override;

        virtual void AttachAsColor(const u8& Index, EFace InFace) override;


        /** Texture interface */

        virtual void Bind() override;
        virtual void Free() override;

        virtual void SetMinFilter(const EMinTextureFilter& Filter) override;
        virtual void SetMagFilter(const EMagTextureFilter& Filter) override;
        virtual void SetWrapSFilter(const EWrapTextureFilter& Filter) override;
        virtual void SetWrapTFilter(const EWrapTextureFilter& Filter) override;
        virtual void SetWrapRFilter(const EWrapTextureFilter& Filter) override;
        virtual void SetBorderColor(const FColor& InColor) override;
        
        virtual void CopyPixels(void* DestBuffer, const u8& MipLevel) override;

      private:
        GLuint glCubemapHandle;
    };

} // namespace lucid::gpu