#include "common/strings.hpp"
#include "common/bytes.hpp"

#include <cstdarg>
#include <string.h>

#include "common/types.hpp"

namespace lucid
{
    FSString EMPTY_STRING { "" };

    /**************************************
    *           Base ANSI String          *
    **************************************/
    
    FString::FString(char* InCString, const u32& InLength)
    {
        CString = InCString;
        UpdateLengthAndCalculateHash(InLength);
    }

    void FString::UpdateLengthAndCalculateHash(const u32& InLength)
    {
        Length = InLength == 0 ? strlen(CString) : InLength;
        Hash = 0;

        for (u64 Idx = 0; Idx < Length; ++Idx)
        {
            Hash = 37 * Hash + CString[Idx];   
        }
    }

    char* FString::GetBytesCopy() const
    {
        return (char*)CopyBytes(CString, Length + 1);
    }

    /**************************************
    *          Dynamic ANSI String        *
    **************************************/

    FDString::FDString(char* InCString, const u32& InLength) : FString(InCString, InLength) {}

    void FDString::Append(const FString& InANSIString)
    {
        Append(*InANSIString, InANSIString.GetLength());
    }

    void FDString::Append(const char* InString, const u64& InStringLength)
    {
        const u32 NewLength = Length + InStringLength;
        char* ConcatBuffer = (char*)malloc(NewLength + 1);
        memcpy(ConcatBuffer, CString, Length);
        memcpy(ConcatBuffer + Length, InString, InStringLength);
        ConcatBuffer[Length + InStringLength] = '\0';
        Free();
        CString = ConcatBuffer;
        UpdateLengthAndCalculateHash(NewLength);        
    }

    void FDString::operator=(const FDString& InRHS)
    {
        CString = InRHS.CString;
        Length = InRHS.Length;
        Hash = InRHS.Hash;
    }

    void FDString::Free() { free(CString); }

    void FDString::Resize(const u32& NewLength)
    {
        char* NewStr = (char*)CopyBytes(CString, Length, NewLength + 1);
        free(CString);
        CString = NewStr;
        UpdateLengthAndCalculateHash(NewLength);
        CString[NewLength] = 0;
    }
    
    /**************************************
    *          Static String            *
    **************************************/

    FSString::FSString(char* InCString, const u32& InLength) : FString(InCString, InLength) {}

    FDString CopyToString(char const* InToCopy, const u32& InStringLength)
    {
        const u32 StrLength = InStringLength == 0 ? strlen(InToCopy) : InStringLength;
        char* CopiedString = (char*)CopyBytes(InToCopy, StrLength, StrLength + 1);
        CopiedString[StrLength] = '\0';
        return FDString { CopiedString, StrLength };
    }

    FDString SPrintf(const char* InFormat, ...)
    {
        static char MsgBuffer[5024];
        va_list Args;
        va_start(Args, InFormat);
        const i32 FormatSize = vsprintf_s(MsgBuffer, 5024, InFormat, Args);
        va_end(Args);
        return CopyToString(MsgBuffer, FormatSize);
    }
} // namespace lucid
