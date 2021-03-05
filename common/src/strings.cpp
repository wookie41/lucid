#include "common/strings.hpp"

#include "stdlib.h"
#include <cassert>
#include <string.h>

namespace lucid
{
    String EMPTY_STRING { "" };

    /**************************************
    *           Base ANSI String          *
    **************************************/
    
    ANSIString::ANSIString(char* InCString, const u32& InLength)
    {
        CString = InCString;
        UpdateLengthAndCalculateHash(InLength);
    }

    void ANSIString::UpdateLengthAndCalculateHash(const u32& InLength)
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

    DString::DString(char* InCString, const u32& InLength) : ANSIString(InCString, InLength) {}

    
    void DString::Append(const ANSIString& InANSIString)
    {
        Append(*InANSIString, InANSIString.GetLength());
    }

    void DString::Append(const char* InString, const u64& InStringLength)
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


    void DString::Free() { free(CString); }

    /**************************************
    *          Static String            *
    **************************************/

    String::String(char* InCString, const u32& InLength) : ANSIString(InCString, InLength) {}

    DString CopyToString(char const* InToCopy, const u32& InStringLength)
    {
        char* CopiedString = (char*)CopyBytes(InToCopy, InStringLength, InStringLength + 1);
        CopiedString[InStringLength] = '\0';
        return DString { CopiedString, InStringLength };
    }

} // namespace lucid
