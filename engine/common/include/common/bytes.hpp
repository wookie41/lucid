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
        char*   Pointer     = nullptr;
        u32     Capacity    = 0;
        u32     Size        = 0;

        void Free()
        {
            assert(Pointer);
            free(Pointer);
            Pointer = nullptr;
        }
    };

    struct FBinaryData
    {
        bool operator!=(const FBinaryData& InRHS) const
        {
            return Pointer != InRHS.Pointer;
        }
        const char* FileName = nullptr;
        u64         Size = 0;
        u64         Offset = 0;         // Offset in the file at which the data is stored (including the CAsset header)
        char*       Pointer = nullptr;

    };

    extern FBinaryData EMPTY_BINARY_DATA;
    
    void* CopyBytes(const char* InToCopy, const u64& InCount, const u64& BufferSize = 0);

    FMemBuffer CreateMemBuffer(const u32& BufferCapacity);
} // namespace lucid
