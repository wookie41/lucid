#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "common/types.hpp"
namespace lucid::platform
{
    /**
    * Called so that the platform system can update itself, meaning: check handles status
    */
    void Update();
    void _UpdateSystem();

    i8 ExecuteCommand(const String& InCommand, const StaticArray<String>& Args);
} // namespace lucid::platform
