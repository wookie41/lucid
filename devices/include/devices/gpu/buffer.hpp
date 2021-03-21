#pragma once

#include <cstdint>

#include "common/types.hpp"

namespace lucid::gpu
{
    enum class EBufferBindPoint : u16
    {
        UNBOUND,
        VERTEX,
        ELEMENT,
        SHADER_STORAGE
    };

    enum class EBufferUsage : u16
    {
        STATIC,
        DYNAMIC,
        COPY_READ,
        COPY_WRITE
    };

    enum EImmutableBufferUsage : u16
    {
        IMM_BUFFER_DYNAMIC = 1,
        IMM_BUFFER_READ = 2,
        IMM_BUFFER_WRITE = 4,
        IMM_BUFFER_PERSISTENT = 8,
        IMM_BUFFER_COHERENT = 16,
        IMM_BUFFER_CLIENT_STORAGE = 32
    };

    enum EBufferAccessPolicy : uint16_t
    {
        BUFFER_READ = 1,
        BUFFER_WRITE = 2,
        BUFFER_PERSISTENT = 4,
        BUFFER_COHERENT = 8
    };

    struct FBufferDescription
    {
        uint32_t size = 0;
        uint32_t offset = 0;
        void* data = nullptr;
    };

    class CBuffer
    {
      public:
        virtual uint32_t GetSize() const = 0;

        virtual void Bind(const EBufferBindPoint& BindPoint) = 0;
        virtual void BindIndexed(const uint32_t& index, const EBufferBindPoint& BindPoint) = 0;

        virtual void Upload(FBufferDescription const* Description) = 0;
        virtual void Download(void* Destination, uint32_t Size = 0, const uint32_t& Offset = 0) = 0;

        virtual void* MemoryMap(const EBufferBindPoint& BindPoint,
                                const EBufferAccessPolicy& AccessPolicy,
                                uint32_t Size = 0,
                                const uint32_t& Offset = 0) = 0;

        virtual void MemoryUnmap() = 0;

        virtual void Free() = 0;

        virtual ~CBuffer() = default;
    };

    CBuffer* CreateBuffer(const FBufferDescription& Description, const EBufferUsage& Usage);
    CBuffer* CreateImmutableBuffer(const FBufferDescription& Description,
                                  const EImmutableBufferUsage& ImmutableBufferUsage);
} // namespace lucid::gpu
