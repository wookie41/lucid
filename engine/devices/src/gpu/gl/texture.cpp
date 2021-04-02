#include "devices/gpu/gl/texture.hpp"
#include "devices/gpu/gl/cubemap.hpp"
#include "devices/gpu/gpu.hpp"

#include <cassert>

#include "devices/gpu/gl/common.hpp"

namespace lucid::gpu
{
    static GLenum GL_MIN_FILTERS_MAPPING[] = { GL_NEAREST,
                                               GL_LINEAR,
                                               GL_NEAREST_MIPMAP_NEAREST,
                                               GL_LINEAR_MIPMAP_NEAREST,
                                               GL_NEAREST_MIPMAP_LINEAR,
                                               GL_LINEAR_MIPMAP_LINEAR };

    static GLenum GL_MAG_FILTERS_MAPPING[] = { GL_NEAREST, GL_LINEAR };
    static GLenum GL_WRAP_FILTERS_MAPPING[] = { GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT };
    static GLenum GL_TEXTURE_DATA_TYPE_MAPPING[] = { GL_UNSIGNED_BYTE, GL_FLOAT, GL_UNSIGNED_INT };
    static GLenum GL_TEXTURE_TARGET_MAPPING[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D };
    static GLenum GL_TEXTURE_DATA_FORMAT[] = {
        GL_RED,    GL_R16F, GL_R32F,    GL_R32UI,   GL_RG,   GL_RG16F,      GL_RG32F,           GL_RGB,          GL_RGB16F,
        GL_RGB32F, GL_RGBA, GL_RGBA16F, GL_RGBA32F, GL_SRGB, GL_SRGB_ALPHA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
    };
    static GLenum GL_TEXTURE_PIXEL_FORMAT[] = { GL_RED, GL_RED_INTEGER, GL_RG, GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL };

#define TO_GL_MIN_FILTER(gl_filters) (GL_MIN_FILTERS_MAPPING[(u8)gl_filters])
#define TO_GL_MAG_FILTER(gl_filters) (GL_MAG_FILTERS_MAPPING[(u8)gl_filters])
#define TO_GL_WRAP_FILTER(gl_filters) (GL_WRAP_FILTERS_MAPPING[(u8)gl_filters])
#define TO_GL_TEXTURE_DATA_TYPE(type) (GL_TEXTURE_DATA_TYPE_MAPPING[static_cast<u8>(type)])
#define TO_GL_TEXTURE_TARGET(type) (GL_TEXTURE_TARGET_MAPPING[static_cast<u8>(type)])
#define TO_GL_TEXTURE_DATA_FORMAT(type) (GL_TEXTURE_DATA_FORMAT[static_cast<u8>(type)])
#define TO_GL_TEXTURE_PIXEL_FORMAT(type) (GL_TEXTURE_PIXEL_FORMAT[static_cast<u8>(type)])

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
                              const ETextureDataType& DataType,
                              const ETextureDataFormat& InDataFormat,
                              const ETexturePixelFormat& InPixelFormat,
                              const int32_t& MipMapLevel,
                              const FANSIString& InName)
    {
        const GLenum GLDataFormat = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat = TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);

        GLuint textureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL,
                                               MipMapLevel,
                                               glm::ivec3{ Width, Height, 0 },
                                               TO_GL_TEXTURE_DATA_TYPE(DataType),
                                               GLDataFormat,
                                               GLPixelFormat,
                                               Data);

        auto* GLTexture = new CGLTexture(textureHandle, TextureType::TWO_DIMENSIONAL, { Width, Height, 0 }, InName);
        GLTexture->SetObjectName();
        return GLTexture;
    }

    CTexture* CreateEmpty2DTexture(const uint32_t& Width,
                                   const uint32_t& Height,
                                   const ETextureDataType& DataType,
                                   const ETextureDataFormat& InDataFormat,
                                   const ETexturePixelFormat& InPixelFormat,
                                   const int32_t& MipMapLevel,
                                   const FANSIString& InName)
    {
        const GLenum GLDataFormat = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat = TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        GLuint textureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL,
                                               MipMapLevel,
                                               glm::ivec3{ Width, Height, 0 },
                                               TO_GL_TEXTURE_DATA_TYPE(DataType),
                                               GLDataFormat,
                                               GLPixelFormat,
                                               nullptr);

        auto* GLTexture = new CGLTexture(textureHandle, TextureType::TWO_DIMENSIONAL, { Width, Height, 0 }, InName);
        GLTexture->SetObjectName();
        return GLTexture;
    }

    CGLTexture::CGLTexture(const GLuint& TextureID,
                           const TextureType& Type,
                           const glm::ivec3& Dimensions,
                           const FANSIString& InName)
    : CGLTexture(TextureID, Dimensions, TO_GL_TEXTURE_TARGET(Type), InName)
    {
    }

    CGLTexture::CGLTexture(const GLuint& TextureID,
                           const glm::ivec3& Dimensions,
                           const GLenum& TextureTaget,
                           const FANSIString& InName)
    : CTexture(InName), glTextureHandle(TextureID), glTextureTarget(TextureTaget), dimensions(Dimensions)
    {
    }

    void CGLTexture::SetObjectName() { SetGLObjectName(GL_TEXTURE, glTextureHandle, Name); }

    glm::ivec3 CGLTexture::GetDimensions() const { return dimensions; }

    void CGLTexture::Bind()
    {
        GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] = this;
        glBindTexture(glTextureTarget, glTextureHandle);
    }

    void CGLTexture::Free() { glDeleteTextures(1, &glTextureHandle); }

    void CGLTexture::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void CGLTexture::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void CGLTexture::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::SetWrapRFilter(const WrapTextureFilter& Filter)
    {
        assert(GPUState->BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::AttachAsColor(const uint8_t& Index)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, glTextureTarget, glTextureHandle, 0);
    }

    void CGLTexture::AttachAsStencil()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, glTextureTarget, glTextureHandle, 0);
    }

    void CGLTexture::AttachAsDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glTextureTarget, glTextureHandle, 0);
    }

    void CGLTexture::AttachAsStencilDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Cubemap

    CCubemap* CreateCubemap(const glm::ivec2& Size,
                            ETextureDataFormat InDataFormat,
                            ETexturePixelFormat InPixelFormat,
                            ETextureDataType DataType,
                            const char* FacesData[6],
                            const FANSIString& InName)
    {
        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

        for (int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         TO_GL_TEXTURE_DATA_FORMAT(InDataFormat),
                         Size.x,
                         Size.y,
                         0,
                         TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat),
                         TO_GL_TEXTURE_DATA_TYPE(DataType),
                         FacesData == nullptr ? nullptr : FacesData[i]);
        }

        // Default sampling parameters

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        auto* GLCubemap = new CGLCubemap(handle, Size, InName);
        GLCubemap->SetObjectName();
        return GLCubemap;
    }

    CGLCubemap::CGLCubemap(const GLuint& Handle, const glm::ivec2& Size, const FANSIString& InName)
    : CCubemap(InName), glCubemapHandle(Handle), size(Size)
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
} // namespace lucid::gpu