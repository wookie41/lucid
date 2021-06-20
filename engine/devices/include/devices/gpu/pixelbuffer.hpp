#pragma once
#include "gpu_object.hpp"

namespace lucid::gpu
{
    class CPixelBuffer : public CGPUObject
    {
      public:
        CPixelBuffer(const FString& InName) : CGPUObject(InName) {}

        virtual void  AsyncReadPixels(const u8& InAttachmentIdx, const u16& InX, const u16& InY, const u16& InWidth, const u16& InHeight, gpu::CFramebuffer* InFramebuffer) = 0;
        virtual bool  IsReady()                                       = 0;
        virtual char* MapBuffer(const EMapMode& InMapMode)            = 0;
        virtual void  UnmapBuffer()                                   = 0;

        virtual ~CPixelBuffer() = default;
    };

    CPixelBuffer* CreatePixelBuffer(const FString& InName, const u32& InSizeInBytes);
} // namespace lucid::gpu
