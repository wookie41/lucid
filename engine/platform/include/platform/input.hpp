#pragma once

#include <cstdint>

#include "common/types.hpp"
#include "SDL2/SDL_keycode.h"

namespace lucid
{
    namespace platform
    {
        class CWindow;
    }

    enum EMouseButton : u8
    {
        LEFT = 1,
        MIDDLE = 2,
        RIGHT = 4
    };

    struct FMousePosition
    {
        bool MouseMoved;
        float X = 0, Y = 0;
        float DeltaX = 0, DeltaY = 0;
    };

    // ActiveWindow - window which callback functions should be called when window-related events occur 
    void ReadEvents(platform::CWindow* ActiveWindow);

    bool IsKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyReleased(const SDL_Keycode& KeyCode);

    bool IsMouseButtonPressed(const EMouseButton& Button);
    bool WasMouseButtonPressed(const EMouseButton& Button);
    bool WasMouseButtonReleased(const EMouseButton& Button);

    FMousePosition GetMousePostion();
    float GetMouseWheelDelta();
} // namespace lucid