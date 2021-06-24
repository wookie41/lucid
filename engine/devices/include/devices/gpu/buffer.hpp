#pragma once

#include <cstdint>

#include "gpu_object.hpp"
#include "common/types.hpp"

namespace lucid::gpu
{
    enum class EBufferBindPoint : u16
    {
        UNBOUND,
        VERTEX,
        ELEMENT,
        SHADER_STORAGE,
        WRITE,
        UNIFORM,
        READ
    };

    enum EBufferUsage : u16
    {
        STREAM_DRAW  = 1,
        STATIC_DRAW  = 2,
        DYNAMIC_DRAW = 4
    };

    enum EImmutableBufferUsage : u16
    {
        IMM_BUFFER_DYNAMIC        = 1,
        IMM_BUFFER_READ           = 2,
        IMM_BUFFER_WRITE          = 4,
        IMM_BUFFER_COHERENT       = 8,
        IMM_BUFFER_CLIENT_STORAGE = 16
    };

    enum EBufferMapPolicy : uint16_t
    {
        BUFFER_READ           = 1,
        BUFFER_WRITE          = 2,
        BUFFER_PERSISTENT     = 4,
        BUFFER_COHERENT       = 8,
        BUFFER_UNSYNCHRONIZED = 16
    };

    struct FBufferDescription
    {
        u32   Size   = 0;
        u32   Offset = 0;
        void* Data   = nullptr;
    };

    class CGPUBuffer : public CGPUObject
    {
      public:
        CGPUBuffer(const FString& InName) : CGPUObject(InName) {}

        virtual uint32_t GetSize() const = 0;

        virtual void Bind(const EBufferBindPoint& BindPoint) = 0;
        virtual void Unbind()                                = 0;

        virtual void BindIndexed(const uint32_t& index, const EBufferBindPoint& BindPoint, const u32& InSize = 0, const u32& InOffset = 0) = 0;
        virtual void BindAsVertexBuffer(const u32& InIndex, const u32& InStride)                                                           = 0;

        virtual void Upload(FBufferDescription const* Description)                              = 0;
        virtual void Download(void* Destination, uint32_t Size = 0, const uint32_t& Offset = 0) = 0;

        virtual void* MemoryMap(const EBufferMapPolicy& InAccessPolicy, const u32& InSize = 0, const u32& InOffset = 0) = 0;
        virtual void  MemoryUnmap()                                                                                     = 0;

        virtual void CopyDataTo(CGPUBuffer* OtherBuffer, const u32& InSize = 0, const u32& InReadOffset = 0, const u32& InWriteOffset = 0) = 0;

        virtual ~CGPUBuffer() = default;
    };

    CGPUBuffer* CreateBuffer(const FBufferDescription& Description, const EBufferUsage& Usage, const FString& InName);

    CGPUBuffer*
    CreateImmutableBuffer(const FBufferDescription& Description, const EImmutableBufferUsage& ImmutableBufferUsage, const FString& InName);
} // namespace lucid::gpu
