#pragma once

#include <cstdint>

namespace lucid::gpu
{
    // we should query the GPU for it
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

    enum class FramebufferBindMode : uint8_t
    {
        READ,
        WRITE,
        READ_WRITE
    };

    class Framebuffer
    {
      public:
        virtual void SetupDrawBuffers(const uint8_t& NumOfBuffers) = 0;
        virtual void EnableDrawBuffer(const uint8_t& BufferIndex, const int8_t& AttachmentIndex) = 0;
        virtual void DisableDrawBuffer(const uint8_t& BufferIndex) = 0;

        virtual bool IsComplete() = 0;

        virtual void Bind(const FramebufferBindMode& Mode) = 0;
        virtual void Unbind() = 0;

        virtual void SetupColorAttachment(const uint32_t& AttachmentIndex, FramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) = 0 ;
        virtual void SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) = 0 ;
        virtual void SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) = 0;


        virtual ~Framebuffer() = default;
        //@TODO Attaching cubemap's faces as color attachments
    };

    Framebuffer* CreateFramebuffer();
    Renderbuffer*
    CreateRenderbuffer(const RenderbufferFormat& Format, const uint32_t& Width, const uint32_t& Height);

} // namespace lucid::gpu
