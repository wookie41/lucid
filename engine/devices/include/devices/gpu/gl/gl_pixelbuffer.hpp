﻿#pragma once

#include <glad/glad.h>

#include "devices/gpu/pixelbuffer.hpp"

namespace lucid::gpu
{
    class CFence;

    class CGLPixelBuffer : public CPixelBuffer
    {
      public:
        CGLPixelBuffer(const FString& InName, const GLuint& InGLHandle);

        virtual void  AsyncReadPixels(const u8&                InAttachmentIdx,
                                      const u16&               InX,
                                      const u16&               InY,
                                      const u16&               InWidth,
                                      const u16&               InHeight,
                                      gpu::CFramebuffer* InFramebuffer) override;
        virtual bool  IsReady() override;
        virtual char* MapBuffer(const EMapMode& InMapMode) override;
        virtual void  UnmapBuffer() override;

        void SetObjectName() override;

        virtual ~CGLPixelBuffer() = default;

        void Free() override;

      private:
        GLenum CurrentBindPoint;
        GLuint  GLHandle  = 0;
        CFence* ReadFence = nullptr;
    };
} // namespace lucid::gpu