#pragma once

#include "bytes.hpp"
#include "common/types.hpp"
namespace lucid
{

    /** Macro which in future will handle cross-platform UTF-16 */
    #define LUCID_TEXT(text) text

    /**
     * Base ANSI string providing a common interface for ANSI string usage
     */
    struct FANSIString
    {
        explicit FANSIString(char* InCString, const u32& InLength = 0);

        char operator[](const u32& InIndex) const;
        const char* operator*() const { return CString; } 

        inline bool operator==(const FANSIString& InRhs) const { return Hash == InRhs.Hash; };
        inline u32 GetLength() const { return Length; }
        inline u64 GetHash() const { return Hash; }

        virtual ~FANSIString() = default;

    protected:
        
        void UpdateLengthAndCalculateHash(const u32& InLength = 0);

        char* CString;
        u64 Hash;
        u32 Length; // doesn't include the null terminator
    };
    
    /**
     * ANSI Null-terminated dynamic string - strings allocated at runtime that can be changed/freed
     */
    struct FDString : public FANSIString
    {
        explicit FDString(char* InCString, const u32& InLength = 0);
        
        void Append(const FANSIString& InString);
        void Append(const char* InString, const u64& InStringLength);
        
        void Free();
    };

    /**
     * ANSI Null-terminated static string - buffers, compile-time strings that don't get freed
     */
    struct FString : public FANSIString
    {
        explicit FString(char* InCString, const u32& InLength = 0);

        FDString CopyToDynamicString() const;
    };
    
    FDString CopyToString(char const* InToCopy, const u32& InStringLength);

    FDString SPrintf(const char* InFormat, ...);
    
    extern FString EMPTY_STRING;
} // namespace lucid
    