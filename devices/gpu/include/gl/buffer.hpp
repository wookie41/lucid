#pragma once

#include "devices/gpu/include/buffer.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLBuffer : public Buffer
    {
      public:
        GLBuffer(const GLuint& BufferHandle, const BufferDescription& Description, const bool& IsImmutable);

        void Bind(const BufferBindPoint& BindPoint) const override;
        void BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint) const override;

        void Upload(BufferDescription const* Description) override;
        void Download(void* Destination, const uint32_t& Offset, int32_t Size) const override;

        void MemoryMap(BufferDescription const* Description, const uint16_t& BufferAccessPolicy) const override;

        void Free() override;

      private:
        bool isImmutable;
        GLuint glBufferHandle;
        BufferDescription description;
    };

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage);
    Buffer* CreateImmutableBuffer(const BufferDescription& Description, const uint16_t& ImmutableBufferUsage);
} // namespace lucid::gpu