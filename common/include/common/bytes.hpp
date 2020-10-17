#pragma once

#include <cstdint>
#include "string.h"

namespace lucid
{
    inline void Zero(void* Address, const uint64_t& NumBytes)
    {
        memset(Address, 0, NumBytes);
    }

    struct MemBuffer
    {
        char* Pointer;
        const uint32_t Capacity;
        uint32_t Length = 0;
    };

    MemBuffer CreateMemBuffer(const uint32_t& BufferCapacity);
} // namespace lucid
