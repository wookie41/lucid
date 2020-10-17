#include "common/strings.hpp"
#include "stdlib.h"
#include <cassert>
#include <string.h>

namespace lucid
{
    // String //

    static inline uint64_t calculateHash(const char const* str)
    {
        uint64_t hash = 0;
        for (size_t idx = 0; idx < strlen(str); ++idx)
            hash = 37 * hash + str[idx];
        return hash;
    }

    String::String(char const* CStr)
    : Hash(calculateHash(CStr)), Length(strlen(CStr)), cStr(CStr)
    {
    }

    String::operator char const*() const { return cStr; }

    char String::operator[](const uint64_t& Index) const
    {
        assert(Index < Length);
        return cStr[Index];
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

    DString::DString(char* CStr) : Hash(calculateHash(CStr)), Length(strlen(CStr)), cStr(CStr)
    {
    }

    DString::operator char*() const { return cStr; }

    char DString::operator[](const uint64_t& Index) const
    {
        assert(Index < Length);
        return cStr[Index];
    }

    DString CopyToDString(char const* ToCopy, uint32_t StrLen)
    {
        StrLen = StrLen == 0 ? strlen(ToCopy) : StrLen;
        char* Copied = (char*)malloc(StrLen + 1);
        memcpy(Copied, ToCopy, StrLen);
        Copied[StrLen] = '\0';
        return { Copied };
    }

    void DString::Free() { free(cStr); }

    DString Concat(const String& Str1, const String& Str2)
    {
        char* concatBuffer = (char*)malloc(Str1.Length + Str2.Length + 1);
        memcpy(concatBuffer, (const char*)Str1, Str1.Length);
        memcpy(concatBuffer + Str1.Length, (const char*)Str2, Str2.Length);
        concatBuffer[Str1.Length + Str2.Length] = '\0';
        return DString{ concatBuffer };
    }
} // namespace lucid
