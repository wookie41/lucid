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
        WRITE = 4,
        PERSISTENT = 8,
        COHERENT = 16,
        CLIENT_STORAGE = 32
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
        virtual uint32_t GetSize() const = 0;

        virtual void Bind(const BufferBindPoint& BindPoint) const = 0;
        virtual void BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint) const = 0;

        virtual void Upload(BufferDescription const* Description) = 0;
        virtual void Download(void* Destination, uint32_t Size = 0, const uint32_t& Offset = 0) const = 0;

        virtual void* MemoryMap(const uint16_t& AccessPolicy, uint32_t Size = 0, const uint32_t& Offset = 0) const = 0;

        virtual void Free() = 0;
    };

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage);
    Buffer* CreateImmutableBuffer(const BufferDescription& Description, const uint16_t& ImmutableBufferUsage);
} // namespace lucid::gpu
