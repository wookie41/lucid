#include "devices/gpu/gl/texture.hpp"
#include "resources/texture.hpp"

#include "devices/gpu/gl/cubemap.hpp"
#include "devices/gpu/gpu.hpp"

#include <cassert>

#include "devices/gpu/gl/common.hpp"

namespace lucid::gpu
{

    // Texture
    static GLuint CreateGLTexture(const TextureType& TextureType,
                                  const GLint& MipMapLevel,
                                  const glm::ivec3& TextureSize,
                                  const GLenum& DataType,
                                  const GLenum& DataFormat,
                                  const GLenum& PixelFormat,
                                  void const* TextureData)
    {
        GLuint textureHandle;
        glGenTextures(1, &textureHandle);
        GLenum textureTarget;
        switch (TextureType)
        {
        case TextureType::ONE_DIMENSIONAL:
            textureTarget = GL_TEXTURE_1D;
            glBindTexture(GL_TEXTURE_1D, textureHandle);
            glTexImage1D(GL_TEXTURE_1D, MipMapLevel, DataFormat, TextureSize.x, 0, PixelFormat, DataType, TextureData);
            break;

        case TextureType::TWO_DIMENSIONAL:
            textureTarget = GL_TEXTURE_2D;
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glTexImage2D(
              GL_TEXTURE_2D, MipMapLevel, DataFormat, TextureSize.x, TextureSize.y, 0, PixelFormat, DataType, TextureData);
            break;

        case TextureType::THREE_DIMENSIONAL:
            textureTarget = GL_TEXTURE_3D;
            glBindTexture(GL_TEXTURE_3D, textureHandle);
            glTexImage3D(GL_TEXTURE_3D,
                         MipMapLevel,
                         DataFormat,
                         TextureSize.x,
                         TextureSize.y,
                         TextureSize.z,
                         0,
                         PixelFormat,
                         DataType,
                         TextureData);
            break;
        }

        if (TextureData)
        {
            glGenerateMipmap(textureTarget);
        }

        glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(textureTarget, 0);

        return textureHandle;
    }

    CTexture* Create2DTexture(void* Data,
                              const uint32_t& Width,
                              const uint32_t& Height,
                              const ETextureDataType& InDataType,
                              const ETextureDataFormat& InDataFormat,
                              const ETexturePixelFormat& InPixelFormat,
                              const int32_t& MipMapLevel,
                              const FString& InName)
    {
        const GLenum GLDataFormat = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat = TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        const GLenum GLDataType = TO_GL_TEXTURE_DATA_TYPE(InDataType);

        GLuint GLTextureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL,
                                                 MipMapLevel,
                                                 glm::ivec3{ Width, Height, 0 },
                                                 GLDataType,
                                                 GLDataFormat,
                                                 GLPixelFormat,
                                                 Data);

        auto* GLTexture = new CGLTexture(GLTextureHandle,
                                         GL_TEXTURE_2D,
                                         { Width, Height, 0 },
                                         InName,
                                         GLPixelFormat,
                                         GLDataType,
                                         Width * Height * GetNumChannels(InPixelFormat) * GetSizeInBytes(InDataType),
                                         InDataType,
                                         InPixelFormat);
        GLTexture->SetObjectName();
        return GLTexture;
    }

    CTexture* CreateEmpty2DTexture(const uint32_t& Width,
                                   const uint32_t& Height,
                                   const ETextureDataType& InDataType,
                                   const ETextureDataFormat& InDataFormat,
                                   const ETexturePixelFormat& InPixelFormat,
                                   const int32_t& MipMapLevel,
                                   const FString& InName)
    {
        const GLenum GLDataFormat = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat = TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        const GLenum GLDataType = TO_GL_TEXTURE_DATA_TYPE(InDataType);

        GLuint GLTextureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL,
                                                 MipMapLevel,
                                                 glm::ivec3{ Width, Height, 0 },
                                                 GLDataType,
                                                 GLDataFormat,
                                                 GLPixelFormat,
                                                 nullptr);

        auto* GLTexture = new CGLTexture(GLTextureHandle,
                                         GL_TEXTURE_2D,
                                         { Width, Height, 0 },
                                         InName,
                                         GLPixelFormat,
                                         GLDataFormat,
                                         Width * Height * GetNumChannels(InPixelFormat) * GetSizeInBytes(InDataType),
                                         InDataType,
                                         InPixelFormat);
        GLTexture->SetObjectName();
        return GLTexture;
    }

    CGLTexture::CGLTexture(const GLuint& InGLTextureID,
                           const GLenum& InGLTextureTarget,
                           const glm::ivec3& InTextureDimensions,
                           const FString& InName,
                           const GLenum& InGLPixelFormat,
                           const GLenum& InGLTextureDataType,
                           const u64& InSizeInBytes,
                           const ETextureDataType& InDataType,
                           const ETexturePixelFormat& InPixelFormat)
    : CTexture(InName, InDataType, InPixelFormat), GLTextureHandle(InGLTextureID), GLTextureTarget(InGLTextureTarget),
      Dimensions(InTextureDimensions), GLPixelFormat(InGLPixelFormat), GLTextureDataType(InGLTextureDataType),
      SizeInBytes(InSizeInBytes)
    {
    }

    void CGLTexture::SetObjectName() { SetGLObjectName(GL_TEXTURE, GLTextureHandle, Name); }

    glm::ivec3 CGLTexture::GetDimensions() const { return Dimensions; }

    void CGLTexture::Bind()
    {
        GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] = this;
        glBindTexture(GLTextureTarget, GLTextureHandle);
    }

    void CGLTexture::Free() { glDeleteTextures(1, &GLTextureHandle); }

    void CGLTexture::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void CGLTexture::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void CGLTexture::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::SetWrapRFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }

    u64 CGLTexture::GetSizeInBytes() const { return SizeInBytes; }

    void CGLTexture::CopyPixels(void* DestBuffer, const u8& MipLevel)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, DestBuffer);
    }

    void CGLTexture::AttachAsColor(const uint8_t& Index)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, GLTextureTarget, GLTextureHandle, 0);
    }

    void CGLTexture::AttachAsStencil()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GLTextureTarget, GLTextureHandle, 0);
    }

    void CGLTexture::AttachAsDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GLTextureTarget, GLTextureHandle, 0);
    }

    void CGLTexture::AttachAsStencilDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, GLTextureHandle, 0);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Cubemap

    CCubemap* CreateCubemap(const glm::ivec2& Size,
                            ETextureDataFormat InDataFormat,
                            ETexturePixelFormat InPixelFormat,
                            ETextureDataType DataType,
                            const resources::CTextureResource* FaceTextures[6],
                            const FString& InName)
    {
        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

        const GLenum GLDataFormat = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        const GLenum GLDataType = TO_GL_TEXTURE_DATA_TYPE(DataType);

        for (int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         GLDataFormat,
                         Size.x,
                         Size.y,
                         0,
                         GLPixelFormat,
                         GLDataType,
                         FaceTextures[i]->TextureData);
        }

        // Default sampling parameters

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        auto* GLCubemap = new CGLCubemap(handle, Size, InName, DataType, InPixelFormat);
        GLCubemap->SetObjectName();
        return GLCubemap;
    }

    CGLCubemap::CGLCubemap(const GLuint& Handle,
                           const glm::ivec2& Size,
                           const FString& InName,
                           const ETextureDataType InTextureDataType,
                           const ETexturePixelFormat InTexturePixelFormat)
    : CCubemap(InName, InTextureDataType, InTexturePixelFormat), glCubemapHandle(Handle), size(Size)
    {
    }

    void CGLCubemap::AttachAsColor(const uint8_t& Index)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this && GPUState->Cubemap == this);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glCubemapHandle, 0);
    }

    void CGLCubemap::AttachAsColor(const uint8_t& Index, EFace InFace)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this && GPUState->Cubemap == this);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint8_t>(InFace),
                               glCubemapHandle,
                               0);
    }

    void CGLCubemap::AttachAsStencil() { glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, glCubemapHandle, 0); }

    void CGLCubemap::AttachAsDepth() { glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glCubemapHandle, 0); }

    void CGLCubemap::AttachAsStencilDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_CUBE_MAP, glCubemapHandle, 0);
    }

    u64 CGLCubemap::GetSizeInBytes() const
    {
        assert(0); //@TODO implement when actually needed
        return -1;
    }

    glm::ivec3 CGLCubemap::GetDimensions() const { return { size.x, size.y, 0 }; }

    void CGLCubemap::SetObjectName() { SetGLObjectName(GL_TEXTURE, glCubemapHandle, Name); }

    glm::ivec2 CGLCubemap::GetSize() const { return size; }

    void CGLCubemap::Bind()
    {
        GPUState->Cubemap = this;
        glBindTexture(GL_TEXTURE_CUBE_MAP, glCubemapHandle);
    }

    void CGLCubemap::Free()
    {
        assert(glCubemapHandle);
        glDeleteTextures(1, &glCubemapHandle);
    }

    void CGLCubemap::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(GPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void CGLCubemap::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(GPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void CGLCubemap::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLCubemap::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLCubemap::SetWrapRFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLCubemap::CopyPixels(void* DestBuffer, const u8& MipLevel)
    {
        assert(0); // @TODO implement when actually needed
    }
} // namespace lucid::gpu