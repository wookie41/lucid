#pragma once

#include <cstdint>

#include "common/types.hpp"
#include "misc/math.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    // we should query the GPU for it
    const u8 MAX_COLOR_ATTACHMENTS = 16;

    class Texture;
    class Framebuffer;
    
    enum class RenderbufferFormat : u8
    {
        DEPTH24_STENCIL8 = 0
    };

    class FramebufferAttachment
    {
      public:
        virtual void Bind() = 0;

        virtual glm::ivec2 GetSize() const = 0;

        virtual void AttachAsColor(const u8& Index) = 0;
        virtual void AttachAsStencil() = 0;
        virtual void AttachAsDepth() = 0;
        virtual void AttachAsStencilDepth() = 0;

        virtual void Free() = 0;

        virtual ~FramebufferAttachment() = default;
    };

    class Renderbuffer : public FramebufferAttachment
    {
      public:
        virtual void Bind() = 0;

        virtual ~Renderbuffer() = default;
    };

    enum class FramebufferBindMode : u8
    {
        READ,
        WRITE,
        READ_WRITE
    };

    class Framebuffer
    {
      public:

        virtual bool IsComplete() = 0;

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const = 0;
        
        virtual void SetupDrawBuffers() = 0;
        virtual void DisableReadWriteBuffers() = 0;

        virtual void Bind(const FramebufferBindMode& Mode) = 0;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, FramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthAttachment(FramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupStencilAttachment(FramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthStencilAttachment(FramebufferAttachment* AttachmentToUse) = 0;

        virtual void Free() = 0;
        
        virtual ~Framebuffer() = default;
        //@TODO Attaching cubemap's faces as color attachments
    };
    
    Framebuffer* CreateFramebuffer();
    Renderbuffer* CreateRenderbuffer(const RenderbufferFormat& Format, const glm::ivec2& Size);

    void BlitFramebuffer(Framebuffer* Source,
                         Framebuffer* Destination,
                         const bool& Color,
                         const bool& Depth,
                         const bool& Stencil,
                         const math::IRectangle& SrcRect,
                         const math::IRectangle& DestRect);

} // namespace lucid::gpu
