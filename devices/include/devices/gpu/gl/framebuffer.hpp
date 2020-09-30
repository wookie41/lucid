#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class GLRenderbuffer : public Renderbuffer
    {
      public:
        GLRenderbuffer(const GLuint& GLRBOHandle,
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
        bool isBound = false;
        RenderbufferFormat format;
        uint32_t width, height;
        GLuint glRBOHandle;
    };

    class GLFramebuffer : public Framebuffer
    {
      public:
        GLFramebuffer(const GLuint& GLFBOHandle);

        virtual void SetupDrawBuffers(const uint8_t& NumOfBuffers) override;

        virtual void EnableDrawBuffer(const uint8_t& BufferIndex, const int8_t& AttachmentIndex) override;
        virtual void DisableDrawBuffer(const uint8_t& BufferIndex) override;

        virtual bool IsComplete() override;

        virtual void Bind(const FramebufferBindMode& Mode) override;
        virtual void Unbind() override;

        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex,
                                          FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) override;

        virtual ~GLFramebuffer();

      private:
        GLuint glFBOHandle;

        bool isBound = 0;
        bool isComplete = false;
        bool isDirty = true;

        int8_t drawBuffersBindings[MAX_COLOR_ATTACHMENTS] = { -1 }; // -1 means that the draw buffer is not used

        FramebufferAttachment* colorAttachments[MAX_COLOR_ATTACHMENTS] = { nullptr };
        FramebufferAttachment* depthAttachment = nullptr;
        FramebufferAttachment* stencilAttachment = nullptr;
        FramebufferAttachment* depthStencilAttachment = nullptr;
    };
} // namespace lucid::gpu
