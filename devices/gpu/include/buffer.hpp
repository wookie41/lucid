#pragma once

#include <cstdint>

namespace lucid::gpu
{
    enum BufferBindPoint
    {
        VERTEX,
        ELEMENT
    };

    enum BufferUsage
    {
        STATIC,
        DYNAMIC
    };

    struct BufferData
    {
        uint32_t size;
        uint32_t offset;
        void* data; // leave null to just allocate the spec
        BufferUsage usage;
    };

    class Buffer
    {
      public:
        virtual void Bind(const BufferBindPoint& BindPoint) = 0;
        virtual void BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint) = 0;

        virtual void Upload(const BufferData& BufferData) = 0;
        virtual void Download(void* Destination) = 0;

        virtual void* MemoryMap() = 0;

        virtual void Free() = 0;
    };

    Buffer* CreateBuffer(const BufferData& BufferData);
} // namespace lucid::gpu
