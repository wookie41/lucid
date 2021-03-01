#include "common/bytes.hpp"

#include <stdlib.h>

namespace lucid
{    
    MemBuffer CreateMemBuffer(const u32& BufferCapacity)
    {
        char* p = (char*)malloc(BufferCapacity);
        Zero(p, BufferCapacity);
        return MemBuffer{ p, BufferCapacity };
    }
} // namespace lucid
