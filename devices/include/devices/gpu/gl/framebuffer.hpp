#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class GLRenderbuffer : public Renderbuffer
    {
      public:
        GLRenderbuffer(const GLuint& GLFBOHandle,
                       const RenderbufferFormat& Format,
                       const uint32_t& Width,
                       const uint32_t& Height);

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void AttachAsColor(const uint8_t& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        virtual ~GLRenderbuffer();

      private:
        RenderbufferFormat format;
        uint32_t Width, Height;
        GLuint glFBOHandle;
    };

    class GLFramebuffer : public Framebuffer
    {
      public:
        GLFramebuffer(const GLuint& GLFBOHandle);

        virtual bool IsComplete() override;

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex, Texture* TextureToUse) override;
        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex, Renderbuffer* RenderbufferToUse) override;

        virtual void SetupDepthAttachment(Texture* TextureToUse) override;
        virtual void SetupDepthAttachment(Renderbuffer* RenderbufferToUse) override;

        virtual void SetupStencilAttachment(Texture* TextureToUse) override;
        virtual void SetupStencilAttachment(Renderbuffer* RenderbufferToUse) override;

        virtual void SetupDepthStencilAttachment(Texture* TextureToUse) override;
        virtual void SetupDepthStencilAttachment(Renderbuffer* RenderbufferToUse) override;

        virtual ~GLFramebuffer() = default;

      private:
        bool isComplete = false;

        FramebufferAttachment* colorAttachments[MAX_COLOR_ATTACHMENTS] = { nullptr };
        FramebufferAttachment* depthAttachment = nullptr;
        FramebufferAttachment* stencilAttachment = nullptr;
        FramebufferAttachment* depthStencilAttachment = nullptr;
    };
} // namespace lucid::gpu
