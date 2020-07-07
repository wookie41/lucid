#pragma once

#include <cstdint>
#include "SDL2/SDL_keycode.h"
#include "common/math.hpp"

namespace lucid
{
    enum MouseButton : uint8_t
    {
        LEFT = 1,
        MIDDLE = 2,
        RIGHT = 4
    };

    void ReadEvents();

    bool IsKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyReleased(const SDL_Keycode& KeyCode);

    bool IsMouseButtonPressed(const MouseButton& Button);
    bool WasMouseButtonPressed(const MouseButton& Button);
    bool WasMouseButtonReleased(const MouseButton& Button);

    math::ivec2 GetMousePostion();
    math::ivec2 GetMousePostionDelta();
    float GetMouseWheelDelta();
} // namespace lucid