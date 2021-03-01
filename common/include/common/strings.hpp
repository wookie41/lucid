#pragma once

#include "common/types.hpp"
namespace lucid
{
    struct String;

    /**
     * Dynamic string - strings allocated at runtime that can be changed/freed
     * We'll need to implement a string area for them later
     */
    struct DString
    {
        DString(char* InCString, const u32& InLength, const u64& InHash);
        DString(char* InCString);
        
        void Append(const String& InStr);
        void Append(const char* InStr, const u32& InLength);

        char operator[](const u64& Index) const;
        inline bool operator==(const DString& rhs) const { return Hash == rhs.Hash; };
        const char* operator*() const { return CString; }

        inline u32 GetLength() const { return Length; }
        inline u64 GetHash() const { return Hash; }

        void Free();

        
      private:
        char* CString;
        u64 Hash;
        u32 Length; // doesn't include the null terminator
    };

    struct String
    {
        // Static string - buffers, compile-time strings that don't get freed
        String(const char* InCString, const u32& InLength = 0);

        const char* operator*() const { return CString; }
        char operator[](const u64& Index) const;
        inline bool operator==(const String& rhs) const { return Hash == rhs.Hash; }
        inline u64 GetHash() const { return Hash; }
        inline u32 GetLength() const { return Length; };
        DString ToDString() const;

      private:
        const char* CString;
        u64 Hash;
        u32 Length; // doesn't include the null terminator
    };
    
    // StrLen without null terminator
    String CopyToString(char const* InToCopy, const u32& InStrLen = 0);

    extern String EMPTY_STRING;
} // namespace lucid
    