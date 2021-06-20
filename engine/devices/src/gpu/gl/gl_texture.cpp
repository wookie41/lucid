#include "devices/gpu/gl/gl_texture.hpp"
#include "devices/gpu/gl/gl_cubemap.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/gl/gl_common.hpp"

#include "resources/texture_resource.hpp"

#include <cassert>

#if DEVELOPMENT
#include "imgui.h"
#include "imgui_internal.h"
#endif

#include "glad/glad.h"

namespace lucid::gpu
{
    // Texture
    static GLuint CreateGLTexture(const ETextureType& TextureType,
                                  const GLint&        MipMapLevel,
                                  const glm::ivec3&   TextureSize,
                                  const GLenum&       DataType,
                                  const GLenum&       DataFormat,
                                  const GLenum&       PixelFormat,
                                  void const*         TextureData)
    {
        GLuint textureHandle;
        glGenTextures(1, &textureHandle);
        GLenum textureTarget;
        switch (TextureType)
        {
        case ETextureType::ONE_DIMENSIONAL:
            textureTarget = GL_TEXTURE_1D;
            glBindTexture(GL_TEXTURE_1D, textureHandle);
            glTexImage1D(GL_TEXTURE_1D, MipMapLevel, DataFormat, TextureSize.x, 0, PixelFormat, DataType, TextureData);
            break;

        case ETextureType::TWO_DIMENSIONAL:
            textureTarget = GL_TEXTURE_2D;
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glTexImage2D(GL_TEXTURE_2D, MipMapLevel, DataFormat, TextureSize.x, TextureSize.y, 0, PixelFormat, DataType, TextureData);
            break;

        case ETextureType::THREE_DIMENSIONAL:
            textureTarget = GL_TEXTURE_3D;
            glBindTexture(GL_TEXTURE_3D, textureHandle);
            glTexImage3D(GL_TEXTURE_3D, MipMapLevel, DataFormat, TextureSize.x, TextureSize.y, TextureSize.z, 0, PixelFormat, DataType, TextureData);
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
        // glBindTexture(textureTarget, 0);

        return textureHandle;
    }

    CTexture* Create2DTexture(void*                      Data,
                              const uint32_t&            Width,
                              const uint32_t&            Height,
                              const ETextureDataType&    InDataType,
                              const ETextureDataFormat&  InDataFormat,
                              const ETexturePixelFormat& InPixelFormat,
                              const int32_t&             MipMapLevel,
                              const FString&             InName)
    {
        const GLenum GLDataFormat  = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat = TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        const GLenum GLDataType    = TO_GL_TEXTURE_DATA_TYPE(InDataType);

        GLuint GLTextureHandle =
          CreateGLTexture(ETextureType::TWO_DIMENSIONAL, MipMapLevel, glm::ivec3{ Width, Height, 0 }, GLDataType, GLDataFormat, GLPixelFormat, Data);

        auto* GLTexture = new CGLTexture(GLTextureHandle,
                                         GL_TEXTURE_2D,
                                         Width,
                                         Height,
                                         InName,
                                         GLPixelFormat,
                                         GLDataType,
                                         Width * Height * GetNumChannels(InPixelFormat) * GetSizeInBytes(InDataType),
                                         InDataType,
                                         InDataFormat,
                                         InPixelFormat);
        GLTexture->SetObjectName();
        return GLTexture;
    }

    CTexture* CreateEmpty2DTexture(const uint32_t&            Width,
                                   const uint32_t&            Height,
                                   const ETextureDataType&    InDataType,
                                   const ETextureDataFormat&  InDataFormat,
                                   const ETexturePixelFormat& InPixelFormat,
                                   const int32_t&             MipMapLevel,
                                   const FString&             InName)
    {
        const GLenum GLDataFormat  = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat = TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        const GLenum GLDataType    = TO_GL_TEXTURE_DATA_TYPE(InDataType);

        GLuint GLTextureHandle = CreateGLTexture(
          ETextureType::TWO_DIMENSIONAL, MipMapLevel, glm::ivec3{ Width, Height, 0 }, GLDataType, GLDataFormat, GLPixelFormat, nullptr);

        auto* GLTexture = new CGLTexture(GLTextureHandle,
                                         GL_TEXTURE_2D,
                                         Width,
                                         Height,
                                         InName,
                                         GLPixelFormat,
                                         GLDataFormat,
                                         Width * Height * GetNumChannels(InPixelFormat) * GetSizeInBytes(InDataType),
                                         InDataType,
                                         InDataFormat,
                                         InPixelFormat);
        GLTexture->SetObjectName();
        return GLTexture;
    }

    CGLTexture::CGLTexture(const GLuint&              InGLTextureID,
                           const GLenum&              InGLTextureTarget,
                           const u32&                 InWidth,
                           const u32&                 InHeight,
                           const FString&             InName,
                           const GLenum&              InGLPixelFormat,
                           const GLenum&              InGLTextureDataType,
                           const u64&                 InSizeInBytes,
                           const ETextureDataType&    InDataType,
                           const ETextureDataFormat&  InTextureDataFormat,
                           const ETexturePixelFormat& InPixelFormat)
    : CTexture(InName, InWidth, InHeight, InDataType, InTextureDataFormat, InPixelFormat), GLTextureHandle(InGLTextureID),
      GLTextureTarget(InGLTextureTarget), GLPixelFormat(InGLPixelFormat), GLTextureDataType(InGLTextureDataType), SizeInBytes(InSizeInBytes)
    {
    }

    void CGLTexture::SetObjectName() { SetGLObjectName(GL_TEXTURE, GLTextureHandle, Name); }

    void CGLTexture::Bind()
    {
        GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] = this;
        glBindTexture(GLTextureTarget, GLTextureHandle);
    }

    void CGLTexture::Free() { glDeleteTextures(1, &GLTextureHandle); }

    void CGLTexture::SetMinFilter(const EMinTextureFilter& Filter)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void CGLTexture::SetMagFilter(const EMagTextureFilter& Filter)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void CGLTexture::SetWrapSFilter(const EWrapTextureFilter& Filter)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::SetWrapTFilter(const EWrapTextureFilter& Filter)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLTexture::SetWrapRFilter(const EWrapTextureFilter& Filter)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glTexParameteri(GLTextureTarget, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }

    u64 CGLTexture::GetSizeInBytes() const { return SizeInBytes; }

    void CGLTexture::CopyPixels(void* DestBuffer, const u8& MipLevel) const
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glGetTexImage(GL_TEXTURE_2D, 0, GLPixelFormat, GLTextureDataType, DestBuffer);
    }

    u64 CGLTexture::GetBindlessHandle()
    {
        if (GLBindlessHandle == 0)
        {
            GLBindlessHandle = glGetTextureHandleARB(GLTextureHandle);
            assert(GLBindlessHandle);
        }
        return GLBindlessHandle;
    }

    bool CGLTexture::IsBindlessTextureResident() const
    {
        return bBindlessTextureResident;
    }

    void CGLTexture::MakeBindlessResident()
    {
        assert(GLBindlessHandle);
        if (!bBindlessTextureResident)
        {
            bBindlessTextureResident = true;
            glMakeTextureHandleResidentARB(GLBindlessHandle);
        }
    }

    void CGLTexture::MakeBindlessNonResident()
    {
        assert(GLBindlessHandle);
        if (bBindlessTextureResident)
        {
            bBindlessTextureResident = false;
            glMakeTextureHandleNonResidentARB(GLBindlessHandle);            
        }
    }

    void CGLTexture::AttachAsColor(const uint8_t& Index)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, GLTextureTarget, GLTextureHandle, 0);
    }

    void CGLTexture::AttachAsStencil() { glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GLTextureTarget, GLTextureHandle, 0); }

    void CGLTexture::AttachAsDepth() { glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GLTextureTarget, GLTextureHandle, 0); }

    void CGLTexture::AttachAsStencilDepth()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, GLTextureHandle, 0);
    }

    void CGLTexture::ImGuiDrawToImage(const ImVec2& InImageSize) const
    {
        ImGui::Image((ImTextureID)GLTextureHandle, InImageSize, { 0, 1 }, { 1, 0 });
    }

    bool CGLTexture::ImGuiImageButton(const ImVec2& InImageSize) const
    {
        return ImGui::ImageButton((ImTextureID)GLTextureHandle, InImageSize, { 0, 1 }, { 1, 0 }, 5);
    }

    void CGLTexture::SetBorderColor(const FColor& InColor)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &InColor.r);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Cubemap
    CCubemap* CreateCubemap(const u32&                   Width,
                            const u32&                   Height,
                            const ETextureDataFormat&    InDataFormat,
                            const ETexturePixelFormat&   InPixelFormat,
                            const ETextureDataType&      DataType,
                            resources::CTextureResource* FaceTextures[6],
                            const FString&               InName,
                            const EMinTextureFilter&     InMinFilter,
                            const EMagTextureFilter&     InMagFilter,
                            const EWrapTextureFilter&    InWrapS,
                            const EWrapTextureFilter&    InWrapT,
                            const EWrapTextureFilter&    InWrapR,
                            const FColor&                InBorderColor)
    {
        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

        const GLenum               GLDataFormat = TO_GL_TEXTURE_DATA_FORMAT(InDataFormat);
        const GLenum GLPixelFormat TO_GL_TEXTURE_PIXEL_FORMAT(InPixelFormat);
        const GLenum               GLDataType = TO_GL_TEXTURE_DATA_TYPE(DataType);

        for (int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         GLDataFormat,
                         FaceTextures && FaceTextures[i] ? FaceTextures[i]->Width : Width,
                         FaceTextures && FaceTextures[i] ? FaceTextures[i]->Height : Height,
                         0,
                         GLPixelFormat,
                         GLDataType,
                         FaceTextures && FaceTextures[i] ? FaceTextures[i]->TextureData : nullptr);
        }

        // Default sampling parameters

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(InMinFilter));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(InMagFilter));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(InWrapS));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(InWrapT));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(InWrapR));
        glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &InBorderColor.r);

        auto* GLCubemap = new CGLCubemap(handle, Width, Height, InName, DataType, InDataFormat, InPixelFormat);
        GLCubemap->SetObjectName();
        return GLCubemap;
    }

    CGLCubemap::CGLCubemap(const GLuint&              Handle,
                           const u32&                 InWidth,
                           const u32&                 InHeight,
                           const FString&             InName,
                           const ETextureDataType&    InTextureDataType,
                           const ETextureDataFormat&  InTextureDataFormat,
                           const ETexturePixelFormat& InTexturePixelFormat)
    : CCubemap(InName, InWidth, InHeight, InTextureDataType, InTextureDataFormat, InTexturePixelFormat), glCubemapHandle(Handle)
    {
    }

    void CGLCubemap::AttachAsColor(const uint8_t& Index)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this && GGPUState->Cubemap == this);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glCubemapHandle, 0);
    }

    void CGLCubemap::AttachAsColor(const uint8_t& Index, EFace InFace)
    {
        assert(GGPUState->BoundTextures[gpu::GGPUInfo.ActiveTextureUnit] == this && GGPUState->Cubemap == this);
        glFramebufferTexture2D(
          GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint8_t>(InFace), glCubemapHandle, 0);
    }

    u64 CGLCubemap::GetBindlessHandle()
    {
        assert(0); // not implemented
        return 0;
    }

    bool CGLCubemap::IsBindlessTextureResident() const
    {
        assert(0); // not implemented
        return false;
    }

    void CGLCubemap::MakeBindlessResident()
    {
        assert(0); // not implemented
    }

    void CGLCubemap::MakeBindlessNonResident()
    {
        assert(0); // not implemented  
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

    void CGLCubemap::SetObjectName() { SetGLObjectName(GL_TEXTURE, glCubemapHandle, Name); }

    void CGLCubemap::Bind()
    {
        GGPUState->Cubemap = this;
        glBindTexture(GL_TEXTURE_CUBE_MAP, glCubemapHandle);
    }

    void CGLCubemap::Free()
    {
        assert(glCubemapHandle);
        glDeleteTextures(1, &glCubemapHandle);
    }

    void CGLCubemap::SetMinFilter(const EMinTextureFilter& Filter)
    {
        assert(GGPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TO_GL_MIN_FILTER(Filter));
    }

    void CGLCubemap::SetMagFilter(const EMagTextureFilter& Filter)
    {
        assert(GGPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TO_GL_MAG_FILTER(Filter));
    }

    void CGLCubemap::SetWrapSFilter(const EWrapTextureFilter& Filter)
    {
        assert(GGPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLCubemap::SetWrapTFilter(const EWrapTextureFilter& Filter)
    {
        assert(GGPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLCubemap::SetWrapRFilter(const EWrapTextureFilter& Filter)
    {
        assert(GGPUState->Cubemap == this);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TO_GL_WRAP_FILTER(Filter));
    }

    void CGLCubemap::CopyPixels(void* DestBuffer, const u8& MipLevel) const
    {
        assert(0); // @TODO implement when actually needed
    }

    void CGLCubemap::SetBorderColor(const FColor& InColor)
    {
        assert(GGPUState->Cubemap == this);
        glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &InColor.r);
    }

} // namespace lucid::gpu