#pragma once

#include "devices/gpu/cubemap.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class CGLCubemap : public CCubemap
    {
      public:
        explicit CGLCubemap(const GLuint& Handle,
                            const glm::ivec2& Size,
                            const FString& InName,
                            const ETextureDataType InTextureDataType,
                            const ETexturePixelFormat InTexturePixelFormat);

        virtual void SetObjectName() override;

        // Framebuffer attachment interface
        virtual glm::ivec2 GetSize() const override;

        virtual void AttachAsColor(const u8& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        virtual u64 GetSizeInBytes() const override;

        virtual void AttachAsColor(const u8& Index, EFace InFace) override;

        ////////////////////////////////////

        // Texture interface

        virtual glm::ivec3 GetDimensions() const override;

        virtual void Bind() override;
        virtual void Free() override;

        virtual void SetMinFilter(const MinTextureFilter& Filter) override;
        virtual void SetMagFilter(const MagTextureFilter& Filter) override;
        virtual void SetWrapSFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapTFilter(const WrapTextureFilter& Filter) override;
        virtual void SetWrapRFilter(const WrapTextureFilter& Filter) override;

        virtual void CopyPixels(void* DestBuffer, const u8& MipLevel) override;

        ////////////////////////

      private:
        GLuint glCubemapHandle;
        const glm::ivec2 size;
    };

} // namespace lucid::gpu