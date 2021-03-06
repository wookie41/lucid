#include "devices/gpu/gl/framebuffer.hpp"

#include "common/log.hpp"
#include "devices/gpu/texture.hpp"
#include "GL/glew.h"
#include "devices/gpu/gpu.hpp"

namespace lucid::gpu
{
    static GLenum RENDER_BUFFER_TYPE_MAPPING[] = { GL_DEPTH24_STENCIL8 };

    static inline void GLBindFramebuffer(Framebuffer* InFramebuffer, const GLuint& InFBOHandle, const FramebufferBindMode& InMode)
    {
        switch (InMode)
        {
        case FramebufferBindMode::READ:
            if (gpu::Info.CurrentReadFramebuffer != InFramebuffer)
            {
                gpu::Info.CurrentReadFramebuffer = InFramebuffer;
                glBindFramebuffer(GL_READ_FRAMEBUFFER, InFBOHandle);
            }
            break;
        case FramebufferBindMode::WRITE:
            if (gpu::Info.CurrentWriteFramebuffer != InFramebuffer)
            {
                gpu::Info.CurrentWriteFramebuffer = InFramebuffer;
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, InFBOHandle);
            }
            break;
        case FramebufferBindMode::READ_WRITE:
            if (gpu::Info.CurrentFramebuffer != InFramebuffer)
            {
                gpu::Info.CurrentFramebuffer = gpu::Info.CurrentWriteFramebuffer = gpu::Info.CurrentReadFramebuffer =
                  InFramebuffer;
                glBindFramebuffer(GL_FRAMEBUFFER, InFBOHandle);
            }
            break;
        }
    }

    Framebuffer* CreateFramebuffer()
    {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);

        return new GLFramebuffer(fbo);
    }

    Renderbuffer* CreateRenderbuffer(const RenderbufferFormat& Format, const glm::ivec2& Size)
    {
        GLenum glRenderbufferType = RENDER_BUFFER_TYPE_MAPPING[static_cast<u8>(Format)];

        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, glRenderbufferType, Size.x, Size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        return new GLRenderbuffer(rbo, Format, Size);
    }

    /////////////////////////////////////
    //        OpenGL framebuffer       //
    /////////////////////////////////////

    GLFramebuffer::GLFramebuffer(const GLuint& GLFBOHandle) : glFBOHandle(GLFBOHandle) {}

    void GLFramebuffer::SetupDrawBuffers()
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

    bool GLFramebuffer::IsComplete()
    {
        assert(gpu::Info.CurrentFramebuffer == this);
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch (Status)
        {
        case GL_FRAMEBUFFER_UNDEFINED:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d is undefined", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d has incomplete aattachment", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d is missing attachment", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d has incomplete draw buffers", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d ha incomplete read buffers", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d is unsupported", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d has incomplete multisample", glFBOHandle);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            LUCID_LOG(LogLevel::WARN, "Framebuffer %d has incomplete layer targets", glFBOHandle);
            break;
        }
        return Status == GL_FRAMEBUFFER_COMPLETE;
    }

    void GLFramebuffer::Bind(const FramebufferBindMode& Mode) { GLBindFramebuffer(this, glFBOHandle, Mode); }

    void GLFramebuffer::SetupColorAttachment(const u32& AttachmentIndex, FramebufferAttachment* AttachmentToUse)
    {
        assert(gpu::Info.CurrentFramebuffer == this && AttachmentIndex < gpu::Info.MaxColorAttachments);
        colorAttachments[AttachmentIndex] = AttachmentToUse;
        AttachmentToUse->AttachAsColor(AttachmentIndex);
    }

    void GLFramebuffer::SetupDepthAttachment(FramebufferAttachment* AttachmentToUse)
    {
        assert(gpu::Info.CurrentFramebuffer == this);
        depthAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsDepth();
    }

    void GLFramebuffer::SetupStencilAttachment(FramebufferAttachment* AttachmentToUse)
    {
        assert(gpu::Info.CurrentFramebuffer == this);
        stencilAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsStencil();
    }

    void GLFramebuffer::SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse)
    {
        assert(gpu::Info.CurrentFramebuffer == this);
        depthStencilAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsStencilDepth();
    }

    void GLFramebuffer::DisableReadWriteBuffers()
    {
        assert(gpu::Info.CurrentFramebuffer == this);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    void GLFramebuffer::Free() {}

    GLRenderbuffer::GLRenderbuffer(const GLuint& GLRBOHandle, const RenderbufferFormat& Format, const glm::ivec2& Size)
    : glRBOHandle(GLRBOHandle), format(Format), size(Size)
    {
    }

    void GLRenderbuffer::Bind()
    {
        if (gpu::Info.CurrentRenderbuffer != this)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, glRBOHandle);
            gpu::Info.CurrentRenderbuffer = this;
        }
    }

    void GLRenderbuffer::AttachAsColor(const u8& Index)
    {
        assert(gpu::Info.CurrentRenderbuffer == this && Index == 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, glRBOHandle);
    }
    void GLRenderbuffer::AttachAsStencil()
    {
        assert(gpu::Info.CurrentRenderbuffer == this);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void GLRenderbuffer::AttachAsDepth()
    {
        assert(gpu::Info.CurrentRenderbuffer == this);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void GLRenderbuffer::AttachAsStencilDepth()
    {
        assert(gpu::Info.CurrentRenderbuffer == this);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void GLRenderbuffer::Free() { glDeleteRenderbuffers(1, &glRBOHandle); }

    /////////////////////////////////////
    //    Default OpenGL framebuffer   //
    /////////////////////////////////////

    GLDefaultFramebuffer::GLDefaultFramebuffer(const u16& InWindowWidth, const u16& InWindowHeight)
    {
        WindowWidth = InWindowWidth;
        WindowHeight = InWindowHeight;
    }

    glm::ivec2 GLDefaultFramebuffer::GetColorAttachmentSize(const u8& Idx) const { return { WindowWidth, WindowHeight }; }

    void GLDefaultFramebuffer::SetupDrawBuffers()
    {
        static const GLenum DefaultColorAttachment = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &DefaultColorAttachment);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

    }

    void GLDefaultFramebuffer::DisableReadWriteBuffers()
    {
        assert(gpu::Info.CurrentFramebuffer == this);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    bool GLDefaultFramebuffer::IsComplete() { return true; }

    void GLDefaultFramebuffer::Bind(const FramebufferBindMode& Mode) { GLBindFramebuffer(this, 0, Mode); }

    void GLDefaultFramebuffer::SetupColorAttachment(const u32& AttachmentIndex, FramebufferAttachment* AttachmentToUse)
    {
        assert(0);
    }

    void GLDefaultFramebuffer::SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) { assert(0); }

    void GLDefaultFramebuffer::SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) { assert(0); }

    void GLDefaultFramebuffer::SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) { assert(0); }

    void GLDefaultFramebuffer::Free()
    {
        // Noop
    }

    void BlitFramebuffer(Framebuffer* Source,
                         Framebuffer* Destination,
                         const bool& Color,
                         const bool& Depth,
                         const bool& Stencil,
                         const misc::IRectangle& SrcRect,
                         const misc::IRectangle& DestRect)
    {
        assert(Source != Destination);

        Source->Bind(FramebufferBindMode::READ);
        Destination->Bind(FramebufferBindMode::WRITE);

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

        glBlitFramebuffer(SrcRect.X, SrcRect.Y, SrcRect.Width, SrcRect.Height, DestRect.X, DestRect.Y, DestRect.Width,
                          DestRect.Height, bufferToBlit, GL_LINEAR);
    }
} // namespace lucid::gpu
