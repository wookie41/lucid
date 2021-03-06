#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class GLRenderbuffer : public Renderbuffer
    {
      public:
        GLRenderbuffer(const GLuint& GLRBOHandle, const RenderbufferFormat& Format, const glm::ivec2& Size);

        virtual void Bind() override;

        virtual glm::ivec2 GetSize() const override{ return size; };

        virtual void AttachAsColor(const u8& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;
        virtual void Free() override;

        virtual ~GLRenderbuffer() = default;

      private:
        RenderbufferFormat format;
        glm::ivec2 size;
        GLuint glRBOHandle;
    };

    class GLDefaultFramebuffer : public Framebuffer
    {
    public:
        GLDefaultFramebuffer(const u16& InWindowWidth, const u16& InWindowHeight);

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const override;

        virtual bool IsComplete() override;

        virtual void SetupDrawBuffers() override;
        virtual void DisableReadWriteBuffers() override;

        virtual void Bind(const FramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void Free() override;

        virtual ~GLDefaultFramebuffer() = default;
    private:
        u16 WindowWidth;
        u16 WindowHeight;
    };

    class GLFramebuffer : public Framebuffer
    {
      public:
        explicit GLFramebuffer(const GLuint& GLFBOHandle);

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const override
        {
            assert(colorAttachments[Idx]);
            return colorAttachments[Idx]->GetSize();
        }
        virtual bool IsComplete() override;

        virtual void SetupDrawBuffers() override;
        virtual void DisableReadWriteBuffers() override;

        virtual void Bind(const FramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void Free() override;

        virtual ~GLFramebuffer() = default;

      private:
        GLuint glFBOHandle;
        glm::ivec2 size;

        FramebufferAttachment* colorAttachments[MAX_COLOR_ATTACHMENTS] = { nullptr };
        FramebufferAttachment* depthAttachment = nullptr;
        FramebufferAttachment* stencilAttachment = nullptr;
        FramebufferAttachment* depthStencilAttachment = nullptr;
    };
} // namespace lucid::gpu