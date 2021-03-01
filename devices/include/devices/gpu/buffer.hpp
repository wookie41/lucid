#pragma once

#include <cstdint>

#include "common/types.hpp"

namespace lucid::gpu
{
    enum class BufferBindPoint : u16
    {
        UNBOUND,
        VERTEX,
        ELEMENT,
        SHADER_STORAGE
    };

    enum class BufferUsage : u16
    {
        STATIC,
        DYNAMIC,
        COPY_READ,
        COPY_WRITE
    };

    enum ImmutableBufferUsage : u16
    {
        IMM_BUFFER_DYNAMIC = 1,
        IMM_BUFFER_READ = 2,
        IMM_BUFFER_WRITE = 4,
        IMM_BUFFER_PERSISTENT = 8,
        IMM_BUFFER_COHERENT = 16,
        IMM_BUFFER_CLIENT_STORAGE = 32
    };

    enum BufferAccessPolicy : uint16_t
    {
        BUFFER_READ = 1,
        BUFFER_WRITE = 2,
        BUFFER_PERSISTENT = 4,
        BUFFER_COHERENT = 8
    };

    struct BufferDescription
    {
        uint32_t size = 0;
        uint32_t offset = 0;
        void* data = nullptr;
    };

    class Buffer
    {
      public:
        virtual uint32_t GetSize() const = 0;

        virtual void Bind(const BufferBindPoint& BindPoint) = 0;
        virtual void BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint) = 0;

        virtual void Upload(BufferDescription const* Description) = 0;
        virtual void Download(void* Destination, uint32_t Size = 0, const uint32_t& Offset = 0) = 0;

        virtual void* MemoryMap(const BufferBindPoint& BindPoint,
                                const BufferAccessPolicy& AccessPolicy,
                                uint32_t Size = 0,
                                const uint32_t& Offset = 0) = 0;

        virtual void MemoryUnmap() = 0;

        virtual void Free() = 0;

        virtual ~Buffer() = default;
    };

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage);
    Buffer* CreateImmutableBuffer(const BufferDescription& Description,
                                  const ImmutableBufferUsage& ImmutableBufferUsage);
} // namespace lucid::gpu
