#pragma once

#include <glad/glad.h>

#include "devices/gpu/pixelbuffer.hpp"

namespace lucid::gpu
{
    class CFence;


    class CGLPixelBuffer : public CPixelBuffer
    {
      public:
        CGLPixelBuffer(const FString& InName, const GLuint& InGLHandle);

        virtual void  AsyncReadPixels(gpu::CTexture const* InTexture) override;
        virtual bool  IsReady() override;
        virtual char* MapBuffer(const EMapMode& InMapMode) override;
        virtual void  UnmapBuffer() override;

        void SetObjectName() override;

        virtual ~CGLPixelBuffer() = default;

      private:
        GLuint  GLHandle     = 0;
        CFence* ReadFence    = nullptr;
    };
} // namespace lucid::gpu
