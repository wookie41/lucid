#include "devices/gpu/gl/textures.hpp"

namespace lucid::gpu
{
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


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

    Texture* Create2DTexture(const void const* TextureData, const math::ivec2& TextureSize, const int32_t MipMapLevel, bool IsTransparent)
    {
        GLenum format = IsTransparent ? GL_RGBA : GL_RGB;
        GLuint textureHandle =
        CreateTexture(TextureType::TWO_DIMENSIONAL, MipMapLevel, { TextureSize.x, TextureSize.y, 0 },
                      GL_UNSIGNED_BYTE, format, format, TextureData);

        return new GLTexture(textureHandle, TextureType::TWO_DIMENSIONAL,
                             { TextureSize.x, TextureSize.y, 0 });
    }

    GLTexture::GLTexture(const GLuint& TextureID, const TextureType& Type, const math::ivec3& Size)
    : glTextureHandle(TextureID), size(Size)
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

    GLTexture::~GLTexture() { glDeleteTextures(1, &glTextureHandle); }

    math::ivec3 GLTexture::GetSize() const { return size; };

} // namespace lucid::gpu