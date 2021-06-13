#include "devices/gpu/gl/gl_framebuffer.hpp"

#include "common/log.hpp"
#include "devices/gpu/texture.hpp"
#include "glad/glad.h"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/gl/gl_common.hpp"

namespace lucid::gpu
{
    static GLenum RENDER_BUFFER_TYPE_MAPPING[] = { GL_DEPTH24_STENCIL8 };

    inline void GLBindFramebuffer(FGPUState* InGPUState,
                                  CFramebuffer* InFramebuffer,
                                  const GLuint& InFBOHandle,
                                  const EFramebufferBindMode& InMode)
    {
        switch (InMode)
        {
        case EFramebufferBindMode::READ:
            if (InGPUState->ReadFramebuffer != InFramebuffer)
            {
                InGPUState->ReadFramebuffer = InFramebuffer;
                glBindFramebuffer(GL_READ_FRAMEBUFFER, InFBOHandle);
            }
            break;
        case EFramebufferBindMode::WRITE:
            if (InGPUState->WriteFramebuffer != InFramebuffer)
            {
                InGPUState->WriteFramebuffer = InFramebuffer;
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, InFBOHandle);
            }
            break;
        case EFramebufferBindMode::READ_WRITE:
            if (InGPUState->Framebuffer != InFramebuffer)
            {
                InGPUState->Framebuffer = InGPUState->WriteFramebuffer = InGPUState->ReadFramebuffer = InFramebuffer;
                glBindFramebuffer(GL_FRAMEBUFFER, InFBOHandle);
            }
            break;
        }
    }

    CFramebuffer* CreateFramebuffer(const FString& InName)
    {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);

        auto* GLFramebuffer = new CGLFramebuffer(fbo, InName);
        GLFramebuffer->SetObjectName();
        return GLFramebuffer;
    }

    CRenderbuffer* CreateRenderbuffer(const ERenderbufferFormat& Format, const glm::ivec2& Size, const FString& InName)
    {
        GLenum glRenderbufferType = RENDER_BUFFER_TYPE_MAPPING[static_cast<u8>(Format)];

        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, glRenderbufferType, Size.x, Size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        auto* GLRenderBuffer = new CGLRenderbuffer(rbo, Format, Size, InName);
        GLRenderBuffer->SetObjectName();
        return GLRenderBuffer;
    }

    /////////////////////////////////////
    //        OpenGL framebuffer       //
    /////////////////////////////////////

    CGLFramebuffer::CGLFramebuffer(const GLuint& GLFBOHandle, const FString& InName)
    : CFramebuffer(InName), glFBOHandle(GLFBOHandle)
    {
    }

    void CGLFramebuffer::SetupDrawBuffers()
    {
        static GLenum DrawBuffers[MAX_COLOR_ATTACHMENTS];
        u32 NumOfBuffers = 0;
        for (u8 AttachmentIdx = 0; AttachmentIdx < MAX_COLOR_ATTACHMENTS; ++AttachmentIdx)
        {
            if (colorAttachments[AttachmentIdx])
            {
                DrawBuffers[NumOfBuffers++] = GL_COLOR_ATTACHMENT0 + AttachmentIdx;
            }
        }

        glDrawBuffers(NumOfBuffers, DrawBuffers);
        //@TODO method for setting the read buffers, as now all ReadPixel operations will use the first attachment
        glReadBuffer(DrawBuffers[0]);
    }

    void CGLFramebuffer::SetObjectName() { SetGLObjectName(GL_FRAMEBUFFER, glFBOHandle, Name); }

    bool CGLFramebuffer::IsComplete()
    {
        assert(GGPUState->Framebuffer == this);
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch (Status)
        {
        case GL_FRAMEBUFFER_UNDEFINED:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d is undefined", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d has incomplete aattachment", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d is missing attachment", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d has incomplete draw buffers", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d ha incomplete read buffers", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d is unsupported", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d has incomplete multisample", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            LUCID_LOG(ELogLevel::WARN, "Framebuffer %d has incomplete layer targets", glFBOHandle);
            break;
        }
        return Status == GL_FRAMEBUFFER_COMPLETE;
    }

    void CGLFramebuffer::Bind(const EFramebufferBindMode& Mode) { GLBindFramebuffer(GGPUState, this, glFBOHandle, Mode); }

    void CGLFramebuffer::SetupColorAttachment(const u32& AttachmentIndex, IFramebufferAttachment* AttachmentToUse)
    {
        assert(GGPUState->Framebuffer == this && AttachmentIndex < gpu::GGPUInfo.MaxColorAttachments);
        colorAttachments[AttachmentIndex] = AttachmentToUse;
        AttachmentToUse->AttachAsColor(AttachmentIndex);
    }

    void CGLFramebuffer::SetupDepthAttachment(IFramebufferAttachment* AttachmentToUse)
    {
        assert(GGPUState->Framebuffer == this);
        depthAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsDepth();
    }

    void CGLFramebuffer::SetupStencilAttachment(IFramebufferAttachment* AttachmentToUse)
    {
        assert(GGPUState->Framebuffer == this);
        stencilAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsStencil();
    }

    void CGLFramebuffer::SetupDepthStencilAttachment(IFramebufferAttachment* AttachmentToUse)
    {
        assert(GGPUState->Framebuffer == this);
        depthStencilAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsStencilDepth();
    }

    void CGLFramebuffer::ReadPixels(const u32& InX, const u32& InY, const u32& InWidth, const u32& InHeight, void* Pixels)
    {
        assert(GGPUState->Framebuffer == this);
        glReadPixels(InX,
                     InY,
                     InWidth,
                     InHeight,
                     TO_GL_TEXTURE_PIXEL_FORMAT(colorAttachments[0]->GetAttachmentPixelFormat()),
                     TO_GL_TEXTURE_DATA_TYPE(colorAttachments[0]->GetAttachmentDataType()),
                     Pixels);
    }

    void CGLFramebuffer::DisableReadWriteBuffers()
    {
        assert(GGPUState->Framebuffer == this);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    void CGLFramebuffer::Free() {}

    CGLRenderbuffer::CGLRenderbuffer(const GLuint& GLRBOHandle,
                                     const ERenderbufferFormat& InFormat,
                                     const glm::ivec2& Size,
                                     const FString& InName)
    : CRenderbuffer(InName), glRBOHandle(GLRBOHandle), Format(InFormat), size(Size)
    {
    }

    void CGLRenderbuffer::SetObjectName() { SetGLObjectName(GL_RENDERBUFFER, glRBOHandle, Name); }

    void CGLRenderbuffer::Bind()
    {
        if (GGPUState->Renderbuffer != this)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, glRBOHandle);
            GGPUState->Renderbuffer = this;
        }
    }

    void CGLRenderbuffer::AttachAsColor(const u8& Index)
    {
        assert(GGPUState->Renderbuffer == this && Index == 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, glRBOHandle);
    }
    void CGLRenderbuffer::AttachAsStencil()
    {
        assert(GGPUState->Renderbuffer == this);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void CGLRenderbuffer::AttachAsDepth()
    {
        assert(GGPUState->Renderbuffer == this);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void CGLRenderbuffer::AttachAsStencilDepth()
    {
        assert(GGPUState->Renderbuffer == this);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    }

    void CGLRenderbuffer::Free() { glDeleteRenderbuffers(1, &glRBOHandle); }

    /////////////////////////////////////
    //    Default OpenGL framebuffer   //
    /////////////////////////////////////

    CGLDefaultFramebuffer::CGLDefaultFramebuffer(const u16& InWindowWidth, const u16& InWindowHeight)
    : CFramebuffer(CopyToString("Default"))
    {
        WindowWidth = InWindowWidth;
        WindowHeight = InWindowHeight;
    }

    void CGLDefaultFramebuffer::SetObjectName() { SetGLObjectName(GL_FRAMEBUFFER, 0, Name); }

    void CGLDefaultFramebuffer::SetupDrawBuffers()
    {
        static const GLenum DefaultColorAttachment = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &DefaultColorAttachment);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    void CGLDefaultFramebuffer::DisableReadWriteBuffers()
    {
        assert(GGPUState->Framebuffer == this);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    bool CGLDefaultFramebuffer::IsComplete() { return true; }

    void CGLDefaultFramebuffer::Bind(const EFramebufferBindMode& Mode) { GLBindFramebuffer(GGPUState, this, 0, Mode); }

    void CGLDefaultFramebuffer::SetupColorAttachment(const u32& AttachmentIndex, IFramebufferAttachment* AttachmentToUse)
    {
        assert(0);
    }

    void CGLDefaultFramebuffer::SetupDepthAttachment(IFramebufferAttachment* AttachmentToUse) { assert(0); }

    void CGLDefaultFramebuffer::SetupStencilAttachment(IFramebufferAttachment* AttachmentToUse) { assert(0); }

    void CGLDefaultFramebuffer::SetupDepthStencilAttachment(IFramebufferAttachment* AttachmentToUse) { assert(0); }

    void CGLDefaultFramebuffer::ReadPixels(const u32& InX, const u32& InY, const u32& InWidth, const u32& InHeight, void* Pixels)
    {
        assert(GGPUState->Framebuffer == this);
        glReadPixels(InX,
             InY,
             InWidth,
             InHeight,
             GL_RGBA,
             GL_UNSIGNED_INT_8_8_8_8,
             Pixels);

    }

    void CGLDefaultFramebuffer::Free()
    {
        // Noop
    }

    void BlitFramebuffer(CFramebuffer* Source,
                         CFramebuffer* Destination,
                         const bool& Color,
                         const bool& Depth,
                         const bool& Stencil,
                         const math::FRectangle& SrcRect,
                         const math::FRectangle& DestRect)
    {
        assert(Source != Destination);

        Source->Bind(EFramebufferBindMode::READ);
        Destination->Bind(EFramebufferBindMode::WRITE);

        GLbitfield bufferToBlit = 0;
        if (Color)
        {
            bufferToBlit |= GL_COLOR_BUFFER_BIT;
        }
        if (Depth)
        {
            bufferToBlit |= GL_DEPTH_BUFFER_BIT;
        }
        if (Stencil)
        {
            bufferToBlit |= GL_STENCIL_BUFFER_BIT;
        }

        glBlitFramebuffer(SrcRect.X,
                          SrcRect.Y,
                          SrcRect.Width,
                          SrcRect.Height,
                          DestRect.X,
                          DestRect.Y,
                          DestRect.Width,
                          DestRect.Height,
                          bufferToBlit,
                          GL_LINEAR);
    }

    void CGLFramebuffer::ImGuiDrawToImage(const ImVec2& InImageSize) const
    {
        colorAttachments[0]->Bind();
        colorAttachments[0]->ImGuiDrawToImage(InImageSize);
    }
} // namespace lucid::gpu
