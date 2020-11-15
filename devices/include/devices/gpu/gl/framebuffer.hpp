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

        virtual void AttachAsColor(const uint8_t& Index) override;
        virtual void AttachAsStencil() override;
        virtual void AttachAsDepth() override;
        virtual void AttachAsStencilDepth() override;

        virtual ~GLRenderbuffer();

      private:
        RenderbufferFormat format;
        glm::ivec2 size;
        GLuint glRBOHandle;
    };

    class GLFramebuffer : public Framebuffer
    {
      public:
        GLFramebuffer(const GLuint& GLFBOHandle);

        virtual glm::ivec2 GetColorAttachmentSize(const uint8_t& Idx = 0) const override
        {
            assert(colorAttachments[Idx]);
            return colorAttachments[Idx]->GetSize();
        }

        virtual void SetupDrawBuffers(const uint8_t& NumOfBuffers) override;

        virtual void EnableDrawBuffer(const uint8_t& BufferIndex, const int8_t& AttachmentIndex) override;
        virtual void DisableDrawBuffer(const uint8_t& BufferIndex) override;

        virtual bool IsComplete() override;

        virtual void Bind(const FramebufferBindMode& Mode) override;

        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex, FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) override;
        virtual void SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) override;

        virtual ~GLFramebuffer();

      private:
        GLuint glFBOHandle;
        glm::ivec2 size;

        bool isComplete = false;
        bool isDirty = true;

        int8_t drawBuffersBindings[MAX_COLOR_ATTACHMENTS] = { -1 }; // -1 means that the draw buffer is not used

        FramebufferAttachment* colorAttachments[MAX_COLOR_ATTACHMENTS] = { nullptr };
        FramebufferAttachment* depthAttachment = nullptr;
        FramebufferAttachment* stencilAttachment = nullptr;
        FramebufferAttachment* depthStencilAttachment = nullptr;
    };
} // namespace lucid::gpu