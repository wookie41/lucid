#pragma once

#include <cstdint>
#include "string.h"
#include "types.hpp"

namespace lucid
{
    inline void Zero(void* Address, const u64& NumBytes)
    {
        memset(Address, 0, NumBytes);
    }

    struct MemBuffer
    {
        char* Pointer;
        const u32 Capacity;
        u32 Length = 0;
    };

    MemBuffer CreateMemBuffer(const u32& BufferCapacity);
} // namespace lucid
