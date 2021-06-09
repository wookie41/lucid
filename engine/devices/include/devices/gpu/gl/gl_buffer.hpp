#pragma once

#include "devices/gpu/buffer.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class CGLBuffer : public CGPUBuffer
    {
      public:
        CGLBuffer(const GLuint& InGLBufferHandle, const FBufferDescription& InBufferDescription, const bool& InbImmutable, const FString& InName);

        virtual uint32_t GetSize() const override;

        virtual void SetObjectName() override;
        
        virtual void Bind(const EBufferBindPoint& BindPoint) override;
        virtual void Unbind() override;
        
        virtual void BindIndexed(const uint32_t& Index, const EBufferBindPoint& BindPoint) override;
        virtual void BindAsVertexBuffer(const u32& InIndex, const u32& InStride) override;
        
        virtual void Upload(FBufferDescription const* Description) override;
        virtual void Download(void* Destination, uint32_t Size, const uint32_t& Offset) override;

        virtual void* MemoryMap(const EBufferAccessPolicy& InAccessPolicy,
                                const u32& InSize = 0,
                                const u32& InOffset = 0) override;
        virtual void MemoryUnmap() override;

        virtual void Free() override;

      private:
        EBufferBindPoint    CurrentBindPoint = EBufferBindPoint::UNBOUND;
        bool                bImmutable : 1;
        GLuint              GLBufferHandle;
        FBufferDescription  BufferDescription;
    };
} // namespace lucid::gpu