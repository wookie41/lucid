#pragma once

#include "gpu_object.hpp"
#include "common/types.hpp"
#include "misc/math.hpp"
#include "glm/glm.hpp"
#include "devices/gpu/texture_enums.hpp"

#if DEVELOPMENT
#include "imgui.h"
#endif


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

    class IFramebufferAttachment
    {
      public:
        virtual void Bind() = 0;

        // virtual glm::ivec2 GetSize() const = 0;

        virtual void AttachAsColor(const u8& Index) = 0;
        virtual void AttachAsStencil() = 0;
        virtual void AttachAsDepth() = 0;
        virtual void AttachAsStencilDepth() = 0;

        virtual ETextureDataType     GetAttachmentDataType() const = 0;
        virtual ETexturePixelFormat  GetAttachmentPixelFormat() const = 0;

        virtual ~IFramebufferAttachment() = default;

#if DEVELOPMENT
        virtual void ImGuiDrawToImage(const ImVec2& InImageSize) const = 0;
#endif  
    };

    class CRenderbuffer : public IFramebufferAttachment, public CGPUObject
    {
      public:
        explicit CRenderbuffer(const FString& InName) : CGPUObject(InName) {}

        virtual void Bind() = 0;

        virtual ETextureDataType     GetAttachmentDataType() const override { assert(0); return ETextureDataType::FLOAT; } //not supported
        virtual ETexturePixelFormat  GetAttachmentPixelFormat() const override { assert(0); return ETexturePixelFormat::RED_INTEGER; }; //not supported

        virtual ~CRenderbuffer() = default;

#if DEVELOPMENT
        virtual void ImGuiDrawToImage(const ImVec2& InImageSize) const override
        {
            // noop
        };
#endif  
        
    };

    enum class EFramebufferBindMode : u8
    {
        READ,
        WRITE,
        READ_WRITE
    };

    class CFramebuffer : public CGPUObject
    {
      public:
        explicit CFramebuffer(const FString& InName) : CGPUObject(InName) {}

        virtual bool IsComplete() = 0;

        virtual void SetupDrawBuffers() = 0;
        virtual void DisableReadWriteBuffers() = 0;

        virtual void Bind(const EFramebufferBindMode& Mode) = 0;

        virtual void SetupColorAttachment(const u32& AttachmentIndex, IFramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthAttachment(IFramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupStencilAttachment(IFramebufferAttachment* AttachmentToUse) = 0;
        virtual void SetupDepthStencilAttachment(IFramebufferAttachment* AttachmentToUse) = 0;

        /** Reads pixels of the first attachment to he specified location */
        virtual void ReadPixels(const u32& InX, const u32& InY, const u32& InWidth, const u32& InHeight, void* Pixels) = 0;

        virtual void Free() = 0;

        virtual ~CFramebuffer() = default;

#if DEVELOPMENT
        virtual void ImGuiDrawToImage(const ImVec2& InImageSize) const = 0;
#endif
        
        //@TODO Attaching cubemap's faces as color attachments
    };

    CFramebuffer* CreateFramebuffer(const FString& InName);
    CRenderbuffer* CreateRenderbuffer(const ERenderbufferFormat& Format, const glm::ivec2& Size, const FString& InName);

    void BlitFramebuffer(CFramebuffer* Source,
                         CFramebuffer* Destination,
                         const bool& Color,
                         const bool& Depth,
                         const bool& Stencil,
                         const math::FRectangle& SrcRect,
                         const math::FRectangle& DestRect);

} // namespace lucid::gpu
