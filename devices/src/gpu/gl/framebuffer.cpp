#include "devices/gpu/gl/framebuffer.hpp"
#include "devices/gpu/texture.hpp"
#include "GL/glew.h"
#include "devices/gpu/gpu.hpp"

namespace lucid::gpu
{
    static GLenum RENDER_BUFFER_TYPE_MAPPING[] = { GL_DEPTH24_STENCIL8 };

    static inline void GLBindFramebuffer(Framebuffer* framebuffer, const GLuint& fboHandle, const FramebufferBindMode& Mode)
    {
        switch (Mode)
        {
        case FramebufferBindMode::READ:
            if (gpu::Info.CurrentReadFramebuffer != framebuffer)
            {
                gpu::Info.CurrentReadFramebuffer = framebuffer;
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fboHandle);
            }
            break;
        case FramebufferBindMode::WRITE:
            if (gpu::Info.CurrentWriteFramebuffer != framebuffer)
            {
                gpu::Info.CurrentWriteFramebuffer = framebuffer;
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboHandle);
            }
            break;
        case FramebufferBindMode::READ_WRITE:
            if (gpu::Info.CurrentFramebuffer != framebuffer)
            {
                gpu::Info.CurrentFramebuffer = gpu::Info.CurrentWriteFramebuffer = gpu::Info.CurrentReadFramebuffer = framebuffer;
                glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
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
        GLenum glRenderbufferType = RENDER_BUFFER_TYPE_MAPPING[static_cast<uint8_t>(Format)];

        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, glRenderbufferType, Size.x, Size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        return new GLRenderbuffer(rbo, Format, Size);
    }

    GLFramebuffer::GLFramebuffer(const GLuint& GLFBOHandle) : glFBOHandle(GLFBOHandle) {}

    void BindDefaultFramebuffer(const FramebufferBindMode& Mode)
    {
        GLBindFramebuffer(nullptr, 0, Mode);
    }

    void GLFramebuffer::SetupDrawBuffers(const uint8_t& NumOfBuffers)
    {
        assert(NumOfBuffers <= MAX_COLOR_ATTACHMENTS);
        static GLenum drawBuffers[MAX_COLOR_ATTACHMENTS];

        for (uint8_t drawBufferIdx = 0; drawBufferIdx < NumOfBuffers; ++drawBufferIdx)
        {
            assert(drawBuffersBindings[drawBufferIdx] >= 0);
            drawBuffers[drawBufferIdx] = GL_COLOR_ATTACHMENT0 + drawBuffersBindings[drawBufferIdx];
        }

        glDrawBuffers(NumOfBuffers, drawBuffers);
    }

    void GLFramebuffer::EnableDrawBuffer(const uint8_t& BufferIndex, const int8_t& AttachmentIndex)
    {
        assert(BufferIndex < MAX_COLOR_ATTACHMENTS);
        drawBuffersBindings[BufferIndex] = AttachmentIndex;
    }

    void GLFramebuffer::DisableDrawBuffer(const uint8_t& BufferIndex)
    {
        assert(BufferIndex < MAX_COLOR_ATTACHMENTS);
        drawBuffersBindings[BufferIndex] = -1;
    }

    bool GLFramebuffer::IsComplete()
    {
        if (isDirty)
        {
            assert(gpu::Info.CurrentFramebuffer == this);
            isComplete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
            isDirty = false;
        }
        return isComplete;
    }

    void GLFramebuffer::Bind(const FramebufferBindMode& Mode) { GLBindFramebuffer(this, glFBOHandle, Mode); }

    void GLFramebuffer::SetupColorAttachment(const uint32_t& AttachmentIndex, FramebufferAttachment* AttachmentToUse)
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


    GLFramebuffer::~GLFramebuffer() { glDeleteFramebuffers(1, &glFBOHandle); }

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

    void GLRenderbuffer::AttachAsColor(const uint8_t& Index)
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

    GLRenderbuffer::~GLRenderbuffer() { glDeleteRenderbuffers(1, &glRBOHandle); }

    void BlitFramebuffer(Framebuffer* Source,
                         Framebuffer* Destination,
                         const bool& Color,
                         const bool& Depth,
                         const bool& Stencil,
                         const misc::IRectangle& SrcRect,
                         const misc::IRectangle& DestRect)
    {
        assert(Source != Destination);

        if (Source == nullptr)
        {
            BindDefaultFramebuffer(FramebufferBindMode::READ);
        }
        else
        {
            Source->Bind(FramebufferBindMode::READ);
        }

        if (Destination == nullptr)
        {
            BindDefaultFramebuffer(FramebufferBindMode::WRITE);
        }
        else
        {
            Destination->Bind(FramebufferBindMode::WRITE);
        }

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
