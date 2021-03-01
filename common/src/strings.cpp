#include "common/strings.hpp"
#include "stdlib.h"
#include <cassert>
#include <string.h>

namespace lucid
{
    // String //
    String EMPTY_STRING { "" };
    
    static u64 CalculateHash(char const* InString, const u32 InStringLength)
    {
        u64 Hash = 0;
        for (u64 Idx = 0; Idx < InStringLength; ++Idx)
        {
            Hash = 37 * Hash + InString[Idx];
            
        }
        return Hash;
    }

    String::String(const char* InCString, const u32& InLength)
    {
        CString = InCString;
        Length = InLength == 0 ? strlen(CString) : InLength;
        Hash = CalculateHash(CString, Length);
    }
    char String::operator[](const u64& Index) const
    {
        assert(Index < Length);
        return CString[Index];
    }

    DString String::ToDString() const
    {
        char* Copied = (char*)malloc(Length + 1);
        memcpy(Copied, CString, Length);
        Copied[Length] = '\0';
        return { Copied, Length, Hash };
    }

    String CopyToString(char const* InToCopy, const u32& InStrLen)
    {
        const u32 StrLen = InStrLen == 0 ? strlen(InToCopy) : InStrLen;
        char* Copied = (char*)malloc(StrLen + 1);
        memcpy(Copied, InToCopy, StrLen);
        Copied[StrLen] = '\0';
        return { Copied, StrLen };
    }

    void DString::Free() { free(CString); }

    DString::DString(char* InCString, const u32& InLength, const u64& InHash)
    {
        CString = InCString;
        Length = InLength;
        Hash = InHash;
    }

    DString::DString(char* InCString)
    {
        CString = InCString;
        Length = strlen(InCString);
        Hash = CalculateHash(CString, Length);
    }
    
    void DString::Append(const char* InStr, const u32& InLength)
    {
        const u32 NewLength = Length +  InLength+ 1;
        char* ConcatBuffer = (char*)malloc(NewLength);
        memcpy(ConcatBuffer, CString, Length);
        memcpy(ConcatBuffer + Length, InStr, InLength);
        ConcatBuffer[Length + InLength] = '\0';
        Free();
        CString = ConcatBuffer;
        Length = NewLength;
        Hash = CalculateHash(CString, Length);

    }
    void DString::Append(const String& InStr)
    {
        Append(*InStr, InStr.GetLength());
    }

} // namespace lucid
