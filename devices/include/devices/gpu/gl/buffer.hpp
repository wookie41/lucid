#pragma once

#include "devices/gpu/buffer.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLBuffer : public Buffer
    {
      public:
        GLBuffer(const GLuint& BufferHandle, const BufferDescription& Description, const bool& IsImmutable);

        virtual uint32_t GetSize() const override;

        virtual void Bind(const BufferBindPoint& BindPoint) override;
        virtual void BindIndexed(const uint32_t& Index, const BufferBindPoint& BindPoint) override;

        virtual void Upload(BufferDescription const* Description) override;
        virtual void Download(void* Destination, uint32_t Size, const uint32_t& Offset) override;

        virtual void* MemoryMap(const BufferBindPoint& BindPoint,
                                const uint16_t& AccessPolicy,
                                uint32_t Size,
                                const uint32_t& Offset) override;
        virtual void* MemoryUnmap() override;

        virtual void Free() override;

      private:
        BufferBindPoint currentBindPoint;
        bool isImmutable;
        GLuint glBufferHandle;
        BufferDescription description;
    };

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage);
    Buffer* CreateImmutableBuffer(const BufferDescription& Description, const uint16_t& ImmutableBufferUsage);
} // namespace lucid::gpu