#include "common/strings.hpp"

#include "stdlib.h"
#include <cassert>
#include <cstdarg>
#include <string.h>

#include "common/types.hpp"

namespace lucid
{
    FString EMPTY_STRING { "" };

    /**************************************
    *           Base ANSI String          *
    **************************************/
    
    FANSIString::FANSIString(char* InCString, const u32& InLength)
    {
        CString = InCString;
        UpdateLengthAndCalculateHash(InLength);
    }

    void FANSIString::UpdateLengthAndCalculateHash(const u32& InLength)
    {
        Length = InLength == 0 ?  strlen(CString) : InLength;
        Hash = 0;

        for (u64 Idx = 0; Idx < Length; ++Idx)
        {
            Hash = 37 * Hash + CString[Idx];   
        }
    }
    
    /**************************************
    *          Dynamic ANSI String        *
    **************************************/

    FDString::FDString(char* InCString, const u32& InLength) : FANSIString(InCString, InLength) {}

    
    void FDString::Append(const FANSIString& InANSIString)
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


    void FDString::Free() { free(CString); }

    /**************************************
    *          Static String            *
    **************************************/

    FString::FString(char* InCString, const u32& InLength) : FANSIString(InCString, InLength) {}

    FDString CopyToString(char const* InToCopy, const u32& InStringLength)
    {
        char* CopiedString = (char*)CopyBytes(InToCopy, InStringLength, InStringLength + 1);
        CopiedString[InStringLength] = '\0';
        return FDString { CopiedString, InStringLength };
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
