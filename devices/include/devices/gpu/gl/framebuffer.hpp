#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class CGLRenderbuffer : public CRenderbuffer
    {
      public:
        CGLRenderbuffer(const GLuint& GLRBOHandle, const ERenderbufferFormat& InFormat, const glm::ivec2& Size);

        virtual void Bind() override;

        virtual glm::ivec2 GetSize() const override{ return size; };

        virtual void AttachAsColor(const u8& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;
        virtual void Free() override;

        virtual ~CGLRenderbuffer() = default;

      private:
        ERenderbufferFormat Format;
        glm::ivec2 size;
        GLuint glRBOHandle;
    };

    class GLDefaultFramebuffer : public CFramebuffer
    {
    public:
        GLDefaultFramebuffer(const u16& InWindowWidth, const u16& InWindowHeight);

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const override;

        virtual bool IsComplete() override;

        virtual void SetupDrawBuffers() override;
        virtual void DisableReadWriteBuffers() override;

        virtual void Bind(const EFramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, CFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(CFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(CFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(CFramebufferAttachment* AttachmentToUse) override;
        virtual void Free() override;

        virtual ~GLDefaultFramebuffer() = default;
    private:
        u16 WindowWidth;
        u16 WindowHeight;
    };

    class CGLFramebuffer : public CFramebuffer
    {
      public:
        explicit CGLFramebuffer(const GLuint& GLFBOHandle);

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const override
        {
            assert(colorAttachments[Idx]);
            return colorAttachments[Idx]->GetSize();
        }
        virtual bool IsComplete() override;

        virtual void SetupDrawBuffers() override;
        virtual void DisableReadWriteBuffers() override;

        virtual void Bind(const EFramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, CFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(CFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(CFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(CFramebufferAttachment* AttachmentToUse) override;
        virtual void Free() override;

        virtual ~CGLFramebuffer() = default;

      private:
        GLuint glFBOHandle;
        glm::ivec2 size;

        CFramebufferAttachment* colorAttachments[MAX_COLOR_ATTACHMENTS] = { nullptr };
        CFramebufferAttachment* depthAttachment = nullptr;
        CFramebufferAttachment* stencilAttachment = nullptr;
        CFramebufferAttachment* depthStencilAttachment = nullptr;
    };
} // namespace lucid::gpu