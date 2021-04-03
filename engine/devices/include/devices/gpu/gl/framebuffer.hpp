#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class CGLRenderbuffer : public CRenderbuffer
    {
      public:
        CGLRenderbuffer(const GLuint& GLRBOHandle,
                        const ERenderbufferFormat& InFormat,
                        const glm::ivec2& Size,
                        const FANSIString& InName);

        void SetObjectName() override;

        virtual void Bind() override;

        virtual glm::ivec2 GetSize() const override { return size; };

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

    class CGLDefaultFramebuffer : public CFramebuffer
    {
      public:
        CGLDefaultFramebuffer(const u16& InWindowWidth, const u16& InWindowHeight);

        void SetObjectName() override;

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const override;

        virtual bool IsComplete() override;

        virtual void SetupDrawBuffers() override;
        virtual void DisableReadWriteBuffers() override;

        virtual void Bind(const EFramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, IFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(IFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(IFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(IFramebufferAttachment* AttachmentToUse) override;

        void ReadPixels(void* Pixels) override;
      
        virtual void Free() override;

        virtual ~CGLDefaultFramebuffer() = default;

      private:
        u16 WindowWidth;
        u16 WindowHeight;
    };

    class CGLFramebuffer : public CFramebuffer
    {
      public:
        CGLFramebuffer(const GLuint& GLFBOHandle, const FANSIString& InName);

        void SetObjectName() override;

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const override
        {
            assert(colorAttachments[Idx]);
            return colorAttachments[Idx]->GetSize();
        }
        virtual bool IsComplete() override;

        virtual void SetupDrawBuffers() override;
        virtual void DisableReadWriteBuffers() override;

        virtual void Bind(const EFramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, IFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(IFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(IFramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(IFramebufferAttachment* AttachmentToUse) override;

        virtual void ReadPixels(void* Pixels) override;

        virtual void Free() override;

        virtual ~CGLFramebuffer() = default;

      private:
        GLuint glFBOHandle;
        IFramebufferAttachment* colorAttachments[MAX_COLOR_ATTACHMENTS] = { nullptr };
        IFramebufferAttachment* depthAttachment = nullptr;
        IFramebufferAttachment* stencilAttachment = nullptr;
        IFramebufferAttachment* depthStencilAttachment = nullptr;
    };
} // namespace lucid::gpu