#pragma once

#include <cstdint>

namespace lucid::gpu
{
    enum class BufferBindPoint : uint16_t
    {
        UNBOUND,
        VERTEX,
        ELEMENT,
        SHADER_STORAGE
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
                                const uint16_t& AccessPolicy,
                                uint32_t Size = 0,
                                const uint32_t& Offset = 0) = 0;

        virtual void MemoryUnmap() = 0;

        virtual void Free() = 0;
    };

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage);
    Buffer* CreateImmutableBuffer(const BufferDescription& Description, const uint16_t& ImmutableBufferUsage);
} // namespace lucid::gpu
