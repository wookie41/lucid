#pragma once

#include <cstdint>

namespace lucid::gpu
{
    enum class BufferBindPoint : uint16_t
    {
        VERTEX,
        ELEMENT
    };

    enum class BufferUsage : uint16_t
    {
        STATIC,
        DYNAMIC,
        COPY_READ,
        COPY_WRITE
    };

    enum class ImmutableBufferUsage : uint16_t
    {
        DYNAMIC = 1,
        READ = 2,
        WRITE = 3,
        PERSISTENT = 4,
        COHERENT = 5,
        CLIENT_STORAGE = 6
    };

    enum class BufferAccessPolicy : uint16_t
    {
        READ = 1,
        WRITE = 2,
        PERSISTENT = 4,
        COHERENT = 8
    };

    struct BufferDescription
    {
        uint32_t size;
        uint32_t offset;
        void* data;
    };

    class Buffer
    {
      public:
        virtual void Bind(const BufferBindPoint& BindPoint) const = 0;
        virtual void BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint) const = 0;

        virtual void Upload(BufferDescription const* Description) = 0;
        virtual void Download(void* Destination, const uint32_t& Offset = 0, int32_t Size = -1) const = 0;

        virtual void MemoryMap(BufferDescription const* Description,
                               const uint16_t& BufferAccessPolicy) const = 0;

        virtual void Free() = 0;
    };

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage);
    Buffer* CreateImmutableBuffer(const BufferDescription& Description, const uint16_t& ImmutableBufferUsage);
} // namespace lucid::gpu
