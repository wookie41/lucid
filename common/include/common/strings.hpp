#pragma once

#include <cstdint>

namespace lucid
{
    struct String
    {
        String(char* CStr);

        void Free();
        operator char*() const;
        char operator[](const uint64_t& Index) const;
        inline bool operator==(const String& rhs) const { return Hash == rhs.Hash; };
        uint64_t Hash;
        uint32_t Length;

        char* CString;
    };

    // StrLen without null terminator
    String CopyString(const char const* ToCopy, uint32_t StrLen = 0);
} // namespace lucid
