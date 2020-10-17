#pragma once

#include <cstdint>

namespace lucid
{
    struct String
    {
        // Static string - buffers, compile-time strings that don't get freed
        String(char const* CStr);

        operator char const*() const;
        char operator[](const uint64_t& Index) const;
        inline bool operator==(const String& rhs) const { return Hash == rhs.Hash; };

        uint64_t Hash;
        uint32_t Length; // doesn't include the null terminator

      private:
        char const* cStr;
    };

    // Dynamic string - strings allocated at runtime that can be changed/freed
    // We'll need to implement a string area for them later
    struct DString
    {
        DString(char* CStr);

        void Free();
        operator char*() const;
        char operator[](const uint64_t& Index) const;
        inline bool operator==(const String& rhs) const { return Hash == rhs.Hash; };

        const uint64_t Hash;
        const uint32_t Length; // doesn't include the null terminator

      private:
        char* cStr;
    };

    // StrLen without null terminator
    String CopyToString(char const* ToCopy, uint32_t StrLen = 0);
    DString CopyToDString(char const* ToCopy, uint32_t StrLen = 0);

    //@TODO Vararg
    DString Concat(const String& Str1, const String& Str2);
} // namespace lucid
