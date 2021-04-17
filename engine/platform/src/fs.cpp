    #include "platform/fs.hpp"

#include <stdio.h>
#include <cstdint>

namespace lucid::platform
{
    // @TODO Unicode path support
    FDString ReadFile(const char* FilePath, const bool& NullTerminate)
    {
        char* retVal = nullptr;
        FILE* fileToRead = fopen(FilePath, "rb");

        if (fileToRead == nullptr)
        {
            return FDString { "" };
        }

        fseek(fileToRead, 0, SEEK_END);

        char* buffer = nullptr;
        int32_t numRead = 0;

        u32 fileSize = ftell(fileToRead);
        if (fileSize == -1)
        {
#ifndef NDEBUG
            printf("Failed to determine size of file '%s'\n", FilePath);
#endif
            goto readFileEnd;
        }

        rewind(fileToRead);

        buffer = new char[fileSize + NullTerminate];
        numRead = fread(buffer, 1, fileSize, fileToRead);

        if (numRead != fileSize)
        {
            delete[] buffer;
            goto readFileEnd;
        }

        if (NullTerminate)
        {
            buffer[fileSize] = '\0';
        }

        retVal = buffer;

    readFileEnd:
        fclose(fileToRead);
        return FDString { retVal } ;
    }

} // namespace lucid::platform
