#pragma once

#include <cstdint>

namespace lucid::gpu
{
    const uint8_t MAX_COLOR_ATTACHMENTS = 16;

    class Texture;

    enum class RenderbufferFormat : uint8_t
    {
        DEPTH24_STENCIL8 = 0
    };

    class FramebufferAttachment
    {
      public:

        virtual void AttachAsColor(const uint8_t& Index) = 0;
        virtual void AttachAsStencil() = 0;
        virtual void AttachAsDepth() = 0;
        virtual void AttachAsStencilDepth() = 0;

        virtual ~FramebufferAttachment() = default;
    };

    class Renderbuffer : public FramebufferAttachment
    {
      public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual ~Renderbuffer() = default;
    };

    class Framebuffer
    {
      public:
        virtual bool IsComplete() = 0;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex, Texture* TextureToUse) = 0;
        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex, Renderbuffer* RenderbufferToUse) = 0;

        virtual void SetupDepthAttachment(Texture* TextureToUse) = 0;
        virtual void SetupDepthAttachment(Renderbuffer* RenderbufferToUse) = 0;

        virtual void SetupStencilAttachment(Texture* TextureToUse) = 0;
        virtual void SetupStencilAttachment(Renderbuffer* RenderbufferToUse) = 0;

        virtual void SetupDepthStencilAttachment(Texture* TextureToUse) = 0;
        virtual void SetupDepthStencilAttachment(Renderbuffer* RenderbufferToUse) = 0;

        virtual ~Framebuffer() = default;
        //@TODO Attaching cubemap's faces as color attachments
    };

    Framebuffer* CreateFramebuffer();
    Renderbuffer* CreateRenderbuffer(const RenderbufferFormat, const uint32_t& Width, const uint32_t& Height);

} // namespace lucid::gpu
