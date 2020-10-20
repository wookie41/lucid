#include "devices/gpu/gl/texture.hpp"
#include "devices/gpu/gpu.hpp"

#include <cassert>

#define TO_GL_MIN_FILTER(gl_filters) (GL_MIN_FILTERS_MAPPING[(uint8_t)gl_filters])
#define TO_GL_MAG_FILTER(gl_filters) (GL_MAG_FILTERS_MAPPING[(uint8_t)gl_filters])
#define TO_GL_WRAP_FILTER(gl_filters) (GL_WRAP_FILTERS_MAPPING[(uint8_t)gl_filters])

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

    static inline GLenum toGLTextureFormat(const TextureFormat& Format)
    {
        switch (Format)
        {
        case TextureFormat::RGB:
            return GL_RGB;
        case TextureFormat::RGBA:
            return GL_RGBA;
        }
    }

    static GLuint CreateGLTexture(const TextureType& TextureType,
                                  const GLint& MipMapLevel,
                                  const glm::ivec3& TextureSize,
                                  const GLenum& DataType,
                                  const GLenum& DataFormat,
                                  const GLenum& InternalFormat,
                                  void const* TextureData)
    {
        GLuint textureHandle;
        glGenTextures(1, &textureHandle);

        switch (TextureType)
        {
        case TextureType::ONE_DIMENSIONAL:
            glBindTexture(GL_TEXTURE_1D, textureHandle);
            glTexImage1D(GL_TEXTURE_1D, MipMapLevel, InternalFormat, TextureSize.x, 0, DataFormat, DataType, TextureData);
            glGenerateMipmap(GL_TEXTURE_1D);

            if (TextureData)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glBindTexture(GL_TEXTURE_1D, 0);
            break;

        case TextureType::TWO_DIMENSIONAL:
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glTexImage2D(GL_TEXTURE_2D, MipMapLevel, InternalFormat, TextureSize.x, TextureSize.y, 0, DataFormat, DataType,
                         TextureData);

            if (TextureData)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case TextureType::THREE_DIMENSIONAL:
            glBindTexture(GL_TEXTURE_3D, textureHandle);
            glTexImage3D(GL_TEXTURE_3D, MipMapLevel, InternalFormat, TextureSize.x, TextureSize.y, TextureSize.z, 0, DataFormat,
                         DataType, TextureData);
            glGenerateMipmap(GL_TEXTURE_3D);

            if (TextureData)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glBindTexture(GL_TEXTURE_3D, 0);
            break;
        }
        return textureHandle;
    }

    Texture* Create2DTexture(void* Data,
                             const uint32_t& Width,
                             const uint32_t& Height,
                             const TextureFormat& Format,
                             const int32_t& MipMapLevel,
                             const bool& PerformGammaCorrection)
    {
        GLenum dataFormat = toGLTextureFormat(Format);
        GLenum internalFormat = dataFormat;

        if (PerformGammaCorrection)
        {
            switch (internalFormat)
            {
            case GL_RGB:
                internalFormat = GL_SRGB;
                break;
            case GL_RGBA:
                internalFormat = GL_SRGB8_ALPHA8;
                break;
            default:
                break;
            }
        }

        GLuint textureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL, MipMapLevel, glm::ivec3{ Width, Height, 0 },
                                               GL_UNSIGNED_BYTE, dataFormat, internalFormat, Data);

        return new GLTexture(textureHandle, TextureType::TWO_DIMENSIONAL, { Width, Height, 0 });
    }

    Texture*
    CreateEmpty2DTexture(const uint32_t& Width, const uint32_t& Height, const TextureFormat& Format, const int32_t& MipMapLevel)
    {
        GLenum dataFormat = toGLTextureFormat(Format);
        GLuint textureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL, MipMapLevel, glm::ivec3{ Width, Height, 0 },
                                               GL_UNSIGNED_BYTE, dataFormat, dataFormat, nullptr);

        return new GLTexture(textureHandle, TextureType::TWO_DIMENSIONAL, { Width, Height, 0 });
    }

    GLTexture::GLTexture(const GLuint& TextureID, const TextureType& Type, const glm::ivec3& Dimensions)
    : glTextureHandle(TextureID), dimensions(Dimensions)
    {
        switch (Type)
        {
        case TextureType::ONE_DIMENSIONAL:
            textureType = GL_TEXTURE_1D;
            break;
        case TextureType::TWO_DIMENSIONAL:
            textureType = GL_TEXTURE_2D;
            break;
        case TextureType::THREE_DIMENSIONAL:
            textureType = GL_TEXTURE_3D;
            break;
        }
    }

    glm::ivec3 GLTexture::GetDimensions() const { return dimensions; }

    void GLTexture::Bind()
    {
        gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] = this;
        assert(textureType == GL_TEXTURE_2D);
        glBindTexture(textureType, glTextureHandle);
    }

    void GLTexture::Free() { glDeleteTextures(1, &glTextureHandle); }

    void GLTexture::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void GLTexture::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void GLTexture::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(textureType, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(textureType, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::AttachAsColor(const uint8_t& Index)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, GL_TEXTURE_2D, glTextureHandle, 0);
    }

    void GLTexture::AttachAsStencil()
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    }

    void GLTexture::AttachAsDepth()
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    }

    void GLTexture::AttachAsStencilDepth()
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    };
} // namespace lucid::gpu