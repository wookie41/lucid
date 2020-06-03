#pragma once

#include "devices/gpu/include/buffer.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLBuffer : public Buffer
    {
      public:
        GLBuffer(const GLuint& BufferHandle);

        void Bind(const BufferBindPoint& BindPoint) override;
        void BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint) override;
        
        void Upload(const BufferData& BufferData) override;
        void Download(void* Destination) override;
        
        void* MemoryMap() override;

        void Free() override;

      private:
        GLuint glBufferHandle;
    };

    Buffer* CreateBuffer(const BufferData& BufferData);
} // namespace lucid::gpu
