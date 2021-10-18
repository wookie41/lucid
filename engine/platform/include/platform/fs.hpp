#pragma once

#include "common/bytes.hpp"

#include "common/strings.hpp"
#include "common/types.hpp"

namespace lucid::platform
{
    FDString   ReadFile(const char* FilePath, const bool& NullTerminate);
    FMemBuffer ReadFileToBuffer(const char* FilePath);
    void       RemoveFile(const char* FilePath);

    /**
     * Adds a listener that is called when files in the directory or the directory itself changes.
     * Returns 0 on success, -1 on error
     */

    typedef void (*DirectoryChangedListener)();

    i8   AddDirectoryListener(const FString& InDirectoryPath, DirectoryChangedListener InListener);
    void RemoveDirectoryListener(const FString& InDirectoryPath, DirectoryChangedListener InListener);
} // namespace lucid::platform
