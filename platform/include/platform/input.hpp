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
        RIGHT =  4
    };

    struct KeyboardState
    {
        uint64_t pressedKeys[4] = { 0 }; // keeps track of which keys are currently held down
        uint64_t clickedKeys[4] = { 0 }; // cleared per frame, only tells which keys were pressed down this frame
        uint64_t releasedKeys[4] = { 0 }; // cleared per frame, only tells which keys were released this frame
    };

    struct MouseState
    {
        math::ivec2 position = { 0, 0 };
        math::ivec2 positionDelta = { 0, 0 };
        float wheelDelta = 0;
        uint8_t pressedButtons = 0;
        uint8_t clickedButtons = 0;
        uint8_t releasedButtons = 0;
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