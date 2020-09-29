#include "devices/gpu/gl/texture.hpp"
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

    // TODO extract this function as a public, graphics-API agnostic part of lucid
    static GLuint CreateTexture(const TextureType& TextureType,
                                const GLint& MipMapLevel,
                                const math::ivec3& TextureSize,
                                const GLenum& DataType,
                                const GLenum& DataFormat,
                                const GLenum& InternalFormat,
                                const void const* TextureData)
    {
        GLuint textureHandle;
        glGenTextures(1, &textureHandle);

        switch (TextureType)
        {
        case TextureType::ONE_DIMENSIONAL:
            glBindTexture(GL_TEXTURE_1D, textureHandle);
            glTexImage1D(GL_TEXTURE_1D, MipMapLevel, InternalFormat, TextureSize.x, 0, DataFormat,
                         DataType, TextureData);
            glGenerateMipmap(GL_TEXTURE_1D);
            glBindTexture(GL_TEXTURE_1D, 0);
            break;

        case TextureType::TWO_DIMENSIONAL:
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glTexImage2D(GL_TEXTURE_2D, MipMapLevel, InternalFormat, TextureSize.x, TextureSize.y,
                         0, DataFormat, DataType, TextureData);

            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case TextureType::THREE_DIMENSIONAL:
            glBindTexture(GL_TEXTURE_3D, textureHandle);
            glTexImage3D(GL_TEXTURE_3D, MipMapLevel, InternalFormat, TextureSize.x, TextureSize.y,
                         TextureSize.z, 0, DataFormat, DataType, TextureData);
            glGenerateMipmap(GL_TEXTURE_3D);
            glBindTexture(GL_TEXTURE_3D, 0);
            break;
        }
        return textureHandle;
    }

    Texture* Create2DTexture(const void const* TextureData,
                             const math::ivec2& TextureDimensions,
                             const int32_t MipMapLevel,
                             bool IsTransparent)
    {
        GLenum format = IsTransparent ? GL_RGBA : GL_RGB;
        GLuint textureHandle = CreateTexture(TextureType::TWO_DIMENSIONAL, MipMapLevel,
                                             { TextureDimensions.x, TextureDimensions.y, 0 },
                                             GL_UNSIGNED_BYTE, format, format, TextureData);

        return new GLTexture(textureHandle, TextureType::TWO_DIMENSIONAL,
                             { TextureDimensions.x, TextureDimensions.y, 0 });
    }

    GLTexture::GLTexture(const GLuint& TextureID, const TextureType& Type, const math::ivec3& Dimensions)
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

    void GLTexture::Bind() { glBindTexture(textureType, glTextureHandle); }

    void GLTexture::Unbind() { glBindTexture(textureType, 0); }

    GLTexture::~GLTexture() { glDeleteTextures(1, &glTextureHandle); }

    math::ivec3 GLTexture::GetDimensions() const { return dimensions; };

    void GLTexture::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(isBound);
        glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void GLTexture::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(isBound);
        glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void GLTexture::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(isBound);
        glTexParameteri(textureType, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(isBound);
        glTexParameteri(textureType, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::AttachAsColor(const uint8_t& Index)
    {
        assert(isBound && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTextureHandle, 0);
    }

    void GLTexture::AttachAsStencil()
    {
        assert(isBound && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    }

    void GLTexture::AttachAsDepth()
    {
        assert(isBound && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    }

    void GLTexture::AttachAsStencilDepth()
    {

        assert(isBound && textureType == GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    };
} // namespace lucid::gpu