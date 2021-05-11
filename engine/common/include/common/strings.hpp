#pragma once

#include "common/types.hpp"

namespace lucid
{

    /** Macro which in future will handle cross-platform UTF-16 */
    #define LUCID_TEXT(text) text

    /**
     * Base string providing a common interface for string usage
     */
    struct FString
    {
    public:
        FString(char* InCString, const u32& InLength = 0);

        char operator[](const u32& InIndex) const;
        char* operator*() const { return CString; } 

        char*   GetBytesCopy() const;
        void    CopyToBuffer(char* InDestination) const;

        inline bool operator==(const FString& InRhs) const { return Hash == InRhs.Hash; };
        inline bool operator!=(const FString& InRhs) const { return Hash != InRhs.Hash; };

        inline u32 GetLength() const { return Length; }
        inline u64 GetHash() const { return Hash; }

        virtual void Free() {}

        virtual ~FString() = default;

    protected:
        
        void UpdateLengthAndCalculateHash(const u32& InLength = 0);

        char* CString;
        u64 Hash;
        u32 Length; // doesn't include the null terminator
    };
    
    /**
     * ANSI Null-terminated dynamic string - strings allocated at runtime that can be changed/freed
     */
    struct FDString : public FString
    {
        FDString(char* InCString = "", const u32& InLength = 0);
        FDString(const FDString& InRhs);

        void Append(const FString& InString);
        void Append(const char* InString, const u64& InStringLength);

        void ReplaceWithBuffer(const char* InBuffer);

        
        void operator=(const FDString& InRHS);

        FDString GetCopy() const;
        void Resize(const u32& NewLength);
        
        virtual void Free() override;
    };

    /**
     * ANSI Null-terminated static string - buffers, compile-time strings that don't get freed
     */
    struct FSString : public FString
    {
        explicit FSString(char* InCString, const u32& InLength = 0);
        FDString CopyToDynamicString() const;
    };
    
    FDString CopyToString(char const* InToCopy, const u32& InStringLength = 0);

    FDString SPrintf(const char* InFormat, ...);
    
    extern FSString EMPTY_STRING;
} // namespace lucid
    