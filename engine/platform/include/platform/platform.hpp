#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "common/types.hpp"
#include "glm/vec2.hpp"

namespace lucid::platform
{
    /**
    * Called so that the platform system can update itself, meaning: check handles status
    */
    void Update();
    void _UpdateSystem();

    i8 ExecuteCommand(const FString& InCommand);

    glm::ivec2 GetDisplaySizeInPixels(const u8& DisplayIndex);
} // namespace lucid::platform
