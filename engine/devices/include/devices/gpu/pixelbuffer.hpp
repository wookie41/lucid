#pragma once
#include "gpu_object.hpp"

namespace lucid::gpu
{
    class CPixelBuffer : public CGPUObject
    {
      public:
        CPixelBuffer(const FString& InName) : CGPUObject(InName) {}

        virtual void  AsyncReadPixels(gpu::CTexture const* InTexture) = 0;
        virtual bool  IsReady()                                       = 0;
        virtual char* MapBuffer(const EMapMode& InMapMode)            = 0;
        virtual void  UnmapBuffer()                                   = 0;

        virtual ~CPixelBuffer() = default;
    };

    CPixelBuffer* CreatePixelBuffer(const FString& InName);
} // namespace lucid::gpu
