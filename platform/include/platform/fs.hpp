#pragma once

#include "common/strings.hpp"
#include "common/types.hpp"

namespace lucid::platform
{    
    DString ReadFile(const ANSIString& FilePath, const bool& NullTerminate);
    
    /**
     * Adds a listener that is called when files in the directory or the directory itself changes.
     * Returns 0 on success, -1 on error
     */

    typedef void (*DirectoryChangedListener)();

    i8 AddDirectoryListener(const ANSIString& InDirectoryPath, DirectoryChangedListener InListener);
    void RemoveDirectoryListener(const ANSIString& InDirectoryPath, DirectoryChangedListener InListener);
} // namespace lucid::platform
