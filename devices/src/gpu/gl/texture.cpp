#include "devices/gpu/gl/texture.hpp"
#include "devices/gpu/gl/cubemap.hpp"
#include "devices/gpu/gpu.hpp"

#include <cassert>

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
    static GLenum GL_TEXTURE_DATA_TYPE_MAPPING[] = { GL_UNSIGNED_BYTE, GL_FLOAT };
    static GLenum GL_TEXTURE_TARGET_MAPPING[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D };
    static GLenum GL_TEXTURE_FORMAT_MAPPING[] = { GL_RGB, GL_RGBA, GL_SRGB, GL_SRGB_ALPHA, GL_RGB16F, GL_DEPTH_COMPONENT };

#define TO_GL_MIN_FILTER(gl_filters) (GL_MIN_FILTERS_MAPPING[(uint8_t)gl_filters])
#define TO_GL_MAG_FILTER(gl_filters) (GL_MAG_FILTERS_MAPPING[(uint8_t)gl_filters])
#define TO_GL_WRAP_FILTER(gl_filters) (GL_WRAP_FILTERS_MAPPING[(uint8_t)gl_filters])
#define TO_GL_TEXTURE_DATA_TYPE(type) (GL_TEXTURE_DATA_TYPE_MAPPING[static_cast<uint8_t>(type)])
#define TO_GL_TEXTURE_TARGET(type) (GL_TEXTURE_TARGET_MAPPING[static_cast<uint8_t>(type)])
#define TO_GL_TEXTURE_FORMAT(type) (GL_TEXTURE_FORMAT_MAPPING[static_cast<uint8_t>(type)])

    // Texture

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
        GLenum textureTarget;
        switch (TextureType)
        {
        case TextureType::ONE_DIMENSIONAL:
            textureTarget = GL_TEXTURE_1D;
            glBindTexture(GL_TEXTURE_1D, textureHandle);
            glTexImage1D(GL_TEXTURE_1D, MipMapLevel, InternalFormat, TextureSize.x, 0, DataFormat, DataType, TextureData);
            break;

        case TextureType::TWO_DIMENSIONAL:
            textureTarget = GL_TEXTURE_2D;
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glTexImage2D(GL_TEXTURE_2D, MipMapLevel, InternalFormat, TextureSize.x, TextureSize.y, 0, DataFormat, DataType, TextureData);
            break;

        case TextureType::THREE_DIMENSIONAL:
            textureTarget = GL_TEXTURE_3D;
            glBindTexture(GL_TEXTURE_3D, textureHandle);
            glTexImage3D(GL_TEXTURE_3D, MipMapLevel, InternalFormat, TextureSize.x, TextureSize.y, TextureSize.z, 0, DataFormat,
                         DataType, TextureData);
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

    Texture* Create2DTexture(void* Data,
                             const uint32_t& Width,
                             const uint32_t& Height,
                             const TextureDataType& DataType,
                             const TextureFormat& Format,
                             const int32_t& MipMapLevel,
                             const bool& PerformGammaCorrection)
    {
        GLenum dataFormat = TO_GL_TEXTURE_FORMAT(Format);
        GLenum internalFormat = TO_GL_TEXTURE_FORMAT(Format);

        if (PerformGammaCorrection)
        {
            switch (internalFormat)
            {
            case GL_RGB:
                internalFormat = GL_SRGB;
                break;
            case GL_RGBA:
                internalFormat = GL_SRGB_ALPHA;
                break;
            default:
                break;
            }
        }

        GLuint textureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL, MipMapLevel, glm::ivec3{ Width, Height, 0 },
                                               TO_GL_TEXTURE_DATA_TYPE(DataType), dataFormat, internalFormat, Data);

        return new GLTexture(textureHandle, TextureType::TWO_DIMENSIONAL, { Width, Height, 0 });
    }

    Texture* CreateEmpty2DTexture(const uint32_t& Width,
                                  const uint32_t& Height,
                                  const TextureDataType& DataType,
                                  const TextureFormat& Format,
                                  const int32_t& MipMapLevel)
    {
        GLenum dataFormat = TO_GL_TEXTURE_FORMAT(Format);
        GLuint textureHandle = CreateGLTexture(TextureType::TWO_DIMENSIONAL, MipMapLevel, glm::ivec3{ Width, Height, 0 },
                                               TO_GL_TEXTURE_DATA_TYPE(DataType), dataFormat, dataFormat, nullptr);

        return new GLTexture(textureHandle, TextureType::TWO_DIMENSIONAL, { Width, Height, 0 });
    }

    GLTexture::GLTexture(const GLuint& TextureID, const TextureType& Type, const glm::ivec3& Dimensions)
    : GLTexture(TextureID, Dimensions, TO_GL_TEXTURE_TARGET(Type))
    {
    }

    GLTexture::GLTexture(const GLuint& TextureID, const glm::ivec3& Dimensions, const GLenum& TextureTaget)
    : glTextureHandle(TextureID), glTextureTarget(TextureTaget), dimensions(Dimensions)
    {
    }

    glm::ivec3 GLTexture::GetDimensions() const { return dimensions; }

    void GLTexture::Bind()
    {
        gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] = this;
        glBindTexture(glTextureTarget, glTextureHandle);
    }

    void GLTexture::Free() { glDeleteTextures(1, &glTextureHandle); }

    void GLTexture::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void GLTexture::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void GLTexture::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::SetWrapRFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this);
        glTexParameteri(glTextureTarget, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }

    void GLTexture::AttachAsColor(const uint8_t& Index)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, glTextureTarget, glTextureHandle, 0);
    }

    void GLTexture::AttachAsStencil()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, glTextureTarget, glTextureHandle, 0);
    }

    void GLTexture::AttachAsDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glTextureTarget, glTextureHandle, 0);
    }

    void GLTexture::AttachAsStencilDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTextureHandle, 0);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Cubemap

    Cubemap* CreateCubemap(const glm::ivec2& Size,
                           TextureFormat InternalFormat,
                           TextureFormat DataFormat,
                           TextureDataType DataType,
                           const char* FacesData[6])
    {
        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

        for (int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, TO_GL_TEXTURE_FORMAT(InternalFormat), Size.x, Size.y, 0,
                         TO_GL_TEXTURE_FORMAT(DataFormat), TO_GL_TEXTURE_DATA_TYPE(DataType), FacesData[i]);
        }

        // Default sampling parameters

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return new GLCubemap(handle, Size);
    }

    GLCubemap::GLCubemap(const GLuint& Handle, const glm::ivec2& Size) : glCubemapHandle(Handle), size(Size) {}

    void GLCubemap::AttachAsColor(const uint8_t& Index)
    {
        assert(gpu::Info.BoundTextures[gpu::Info.ActiveTextureUnit] == this && gpu::Info.CurrentCubemap == this);
        for (uint8_t i = 0; i < 6; ++i)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, glCubemapHandle, 0);
        }
    }

    void GLCubemap::AttachAsStencil()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_CUBE_MAP, glCubemapHandle, 0);
    }

    void GLCubemap::AttachAsDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP, glCubemapHandle, 0);
    }

    void GLCubemap::AttachAsStencilDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_CUBE_MAP, glCubemapHandle, 0);
    }

    glm::ivec3 GLCubemap::GetDimensions() const { return { size.x, size.y, 0 }; }

    glm::ivec2 GLCubemap::GetSize() const { return size; }

    void GLCubemap::Bind()
    {
        if (gpu::Info.CurrentCubemap != this)
        {
            gpu::Info.CurrentCubemap = this;
            glBindTexture(GL_TEXTURE_CUBE_MAP, glCubemapHandle);
        }
    }

    void GLCubemap::Free()
    {
        assert(glCubemapHandle);
        glDeleteTextures(1, &glCubemapHandle);
    }

    void GLCubemap::SetMinFilter(const MinTextureFilter& Filter)
    {
        assert(gpu::Info.CurrentCubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void GLCubemap::SetMagFilter(const MagTextureFilter& Filter)
    {
        assert(gpu::Info.CurrentCubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void GLCubemap::SetWrapSFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.CurrentCubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void GLCubemap::SetWrapTFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.CurrentCubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void GLCubemap::SetWrapRFilter(const WrapTextureFilter& Filter)
    {
        assert(gpu::Info.CurrentCubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }
} // namespace lucid::gpu