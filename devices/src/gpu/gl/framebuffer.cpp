#include "devices/gpu/gl/framebuffer.hpp"
#include "devices/gpu/texture.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    static GLenum RENDER_BUFFER_TYPE_MAPPING[] = { GL_DEPTH24_STENCIL8 };
    static GLenum glFramebufferModes[] = { GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER };

    Framebuffer* CreateFramebuffer()
    {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);

        return new GLFramebuffer(fbo);
    }

    Renderbuffer* CreateRenderbuffer(const RenderbufferFormat& Format, const uint32_t& Width, const uint32_t& Height)
    {
        GLenum glRenderbufferType = RENDER_BUFFER_TYPE_MAPPING[static_cast<uint8_t>(Format)];

        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, glRenderbufferType, Width, Height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        return new GLRenderbuffer(rbo, Format, Width, Height);
    }

    GLFramebuffer::GLFramebuffer(const GLuint& GLFBOHandle) : glFBOHandle(GLFBOHandle) {}

    void BindDefaultFramebuffer(const FramebufferBindMode& Mode)
    {
        glBindFramebuffer(glFramebufferModes[static_cast<uint8_t>(Mode)], 0);
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
            assert(isBound);
            isComplete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
            isDirty = false;
        }
        return isComplete;
    }

    void GLFramebuffer::Bind(const FramebufferBindMode& Mode)
    {
        if (!isBound)
        {
            isBound = true;
            glBindFramebuffer(glFramebufferModes[static_cast<uint8_t>(Mode)], glFBOHandle);
        }
    }
    void GLFramebuffer::Unbind() { isBound = false; }


    void GLFramebuffer::SetupColorAttachment(const uint32_t& AttachmentIndex, FramebufferAttachment* AttachmentToUse)
    {
        assert(isBound && AttachmentIndex < MAX_COLOR_ATTACHMENTS);
        colorAttachments[AttachmentIndex] = AttachmentToUse;
        AttachmentToUse->AttachAsColor(AttachmentIndex);
    }

    void GLFramebuffer::SetupDepthAttachment(FramebufferAttachment* AttachmentToUse)
    {
        assert(isBound);
        depthAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsDepth();
    }

    void GLFramebuffer::SetupStencilAttachment(FramebufferAttachment* AttachmentToUse)
    {
        assert(isBound);
        stencilAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsStencil();
    }

    void GLFramebuffer::SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse)
    {
        assert(isBound);
        depthStencilAttachment = AttachmentToUse;
        AttachmentToUse->AttachAsStencilDepth();
    }

    GLFramebuffer::~GLFramebuffer() { glDeleteFramebuffers(1, &glFBOHandle); }

    GLRenderbuffer::GLRenderbuffer(const GLuint& GLRBOHandle,
                                   const RenderbufferFormat& Format,
                                   const uint32_t& Width,
                                   const uint32_t& Height)
    : glRBOHandle(GLRBOHandle), format(Format), width(Width), height(Height)
    {
    }

    void GLRenderbuffer::Bind()
    {
        if (!isBound)
        {
            isBound = true;
            glBindRenderbuffer(GL_RENDERBUFFER, glRBOHandle);
        }
    }
    void GLRenderbuffer::Unbind() { isBound = false; }

    void GLRenderbuffer::AttachAsColor(const uint8_t& Index)
    {
        assert(isBound && Index == 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, glRBOHandle);
    }
    void GLRenderbuffer::AttachAsStencil()
    {
        assert(isBound);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void GLRenderbuffer::AttachAsDepth()
    {
        assert(isBound);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    void GLRenderbuffer::AttachAsStencilDepth()
    {
        assert(isBound);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBOHandle);
    };

    GLRenderbuffer::~GLRenderbuffer() { glDeleteRenderbuffers(1, &glRBOHandle); }

} // namespace lucid::gpu
