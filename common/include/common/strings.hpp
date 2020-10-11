#pragma once

#include <cstdint>

namespace lucid
{
    struct String
    {
        // Static string - buffers, compile-time strings that don't get freed //
        String(char const* CStr);

        operator char const*() const;
        char operator[](const uint64_t& Index) const;
        inline bool operator==(const String& rhs) const { return Hash == rhs.Hash; };
        uint64_t Hash;
        uint32_t Length;

        char const* CString;
    };

    // Dynamic string - strings allocated at runtime that can be changed/freed //
    struct DString
    {
        DString(char* CStr);

        void Free();
        operator char*() const;
        char operator[](const uint64_t& Index) const;
        inline bool operator==(const String& rhs) const { return Hash == rhs.Hash; };
        uint64_t Hash;
        uint32_t Length;

        char* CString;
    };

    // StrLen without null terminator
    String CopyToString(char const* ToCopy, uint32_t StrLen = 0);
    DString CopyToDString(char const* ToCopy, uint32_t StrLen = 0);
} // namespace lucid
