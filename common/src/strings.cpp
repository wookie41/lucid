#include "common/strings.hpp"
#include "stdlib.h"
#include <cassert>
#include <string.h>

namespace lucid
{
    // String //

    String::String(char const* CStr)
    {
        CString = CStr;

        Hash = 0;

        for (Length = 0; CStr[Length] != '\0'; ++Length)
            Hash = 37 * Hash + CStr[Length];
    }

    String::operator char const*() const { return CString; }

    char String::operator[](const uint64_t& Index) const
    {
        assert(Index < Length);
        return CString[Index];
    }

    String CopyToString(char const* ToCopy, uint32_t StrLen)
    {
        StrLen = StrLen == 0 ? strlen(ToCopy) : StrLen;
        char* Copied = (char*)malloc(StrLen + 1);
        memcpy(Copied, ToCopy, StrLen);
        Copied[StrLen] = '\0';
        return { Copied };
    }

    // DString //

    DString::DString(char* CStr)
    {
        CString = CStr;

        Hash = 0;

        for (Length = 0; CStr[Length] != '\0'; ++Length)
            Hash = 37 * Hash + CStr[Length];
    }

    DString::operator char*() const { return CString; }

    char DString::operator[](const uint64_t& Index) const
    {
        assert(Index < Length);
        return CString[Index];
    }

    void DString::Free() { free(CString); }

    DString CopyToDString(char const* ToCopy, uint32_t StrLen)
    {
        StrLen = StrLen == 0 ? strlen(ToCopy) : StrLen;
        char* Copied = (char*)malloc(StrLen + 1);
        memcpy(Copied, ToCopy, StrLen);
        Copied[StrLen] = '\0';
        return { Copied };
    }

} // namespace lucid
