#pragma once

#include <cstdint>
#include "SDL2/SDL_keycode.h"

namespace lucid
{
    struct KeyboardState
    {
        uint64_t pressedKeys[4] = { 0 }; // keeps track of which keys are currently held down
        uint64_t clickedKeys[4] = { 0 }; // cleared per frame, only tells which keys were pressed down this frame
        uint64_t releasedKeys[4] = { 0 }; // cleared per frame, only tells which keys were released this frame
    };

    void ReadEvents();
    bool IsKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyReleased(const SDL_Keycode& KeyCode);
} // namespace lucid