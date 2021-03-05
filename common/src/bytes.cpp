#include "common/bytes.hpp"

#include <stdlib.h>

#include "devices/gpu/buffer.hpp"

namespace lucid
{
    void* CopyBytes(const char* InToCopy, const u64& InCount, const u64& BufferSize)
    {
        assert(InToCopy);

        void* Copied = malloc(BufferSize == 0 ? InCount : BufferSize);
        memcpy(Copied, InToCopy, InCount);
        return Copied;        
    }

    MemBuffer CreateMemBuffer(const u32& BufferCapacity)
    {
        char* p = (char*)malloc(BufferCapacity);
        Zero(p, BufferCapacity);
        return MemBuffer{ p, BufferCapacity };
    }
} // namespace lucid
