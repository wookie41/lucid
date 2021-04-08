#pragma once

#include "types.hpp"

namespace lucid
{
    inline void Zero(void* Address, const u64& NumBytes)
    {
        memset(Address, 0, NumBytes);
    }

    struct FMemBuffer
    {
        char* Pointer;
        const u32 Capacity;
        u32 Length = 0;
    };

    struct FBinaryData
    {
        bool operator!=(const FBinaryData& InRHS) const
        {
            return Pointer != InRHS.Pointer;
        }
        char*   Pointer = nullptr;
        u64     Size = 0;
    };

    extern FBinaryData EMPTY_BINARY_DATA;
    
    void* CopyBytes(const char* InToCopy, const u64& InCount, const u64& BufferSize = 0);

    FMemBuffer CreateMemBuffer(const u32& BufferCapacity);
} // namespace lucid
