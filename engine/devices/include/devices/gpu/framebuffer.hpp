#pragma once

#include <cstdint>


#include "gpu_object.hpp"
#include "common/types.hpp"
#include "misc/math.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    // we should query the GPU for it
    const u8 MAX_COLOR_ATTACHMENTS = 16;

    class CTexture;
    class CFramebuffer;
    
    enum class ERenderbufferFormat : u8
    {
        DEPTH24_STENCIL8 = 0
    };

    class CFramebufferAttachment
    {
      public:
        
        virtual void Bind() = 0;

        virtual glm::ivec2 GetSize() const = 0;

        virtual void AttachAsColor(const u8& Index) = 0;
        virtual void AttachAsStencil() = 0;
        virtual void AttachAsDepth() = 0;
        virtual void AttachAsStencilDepth() = 0;

        virtual ~CFramebufferAttachment() = default;
    };

    class CRenderbuffer : public CFramebufferAttachment, public CGPUObject
    {
      public:
        
        explicit CRenderbuffer(const FANSIString& InName) : CGPUObject(InName) {}
        
        virtual void Bind() = 0;

        virtual ~CRenderbuffer() = default;
    };

    enum class EFramebufferBindMode : u8
    {
        READ,
        WRITE,
        READ_WRITE
    };

    class CFramebuffer: public CGPUObject
    {
      public:
        explicit CFramebuffer(const FANSIString& InName) : CGPUObject(InName) {}
        
        virtual bool IsComplete() = 0;

        virtual glm::ivec2 GetColorAttachmentSize(const u8& Idx = 0) const = 0;
        
        virtual void SetupDrawBuffers() = 0;
        virtual void DisableReadWriteBuffers() = 0;

        virtual void Bind(const EFramebufferBindMode& Mode) = 0;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, CFramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthAttachment(CFramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupStencilAttachment(CFramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthStencilAttachment(CFramebufferAttachment* AttachmentToUse) = 0;

        virtual void Free() = 0;
        
        virtual ~CFramebuffer() = default;
        //@TODO Attaching cubemap's faces as color attachments
    };
    
    CFramebuffer* CreateFramebuffer(const FANSIString& InName);
    CRenderbuffer* CreateRenderbuffer(const ERenderbufferFormat& Format, const glm::ivec2& Size, const FANSIString& InName);

    void BlitFramebuffer(CFramebuffer* Source,
                         CFramebuffer* Destination,
                         const bool& Color,
                         const bool& Depth,
                         const bool& Stencil,
                         const math::FRectangle& SrcRect,
                         const math::FRectangle& DestRect);

} // namespace lucid::gpu
