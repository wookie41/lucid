#pragma once

#include <cstdint>
#include "SDL2/SDL_keycode.h"

namespace lucid
{
    namespace platform
    {
        class Window;
    }

    enum MouseButton : uint8_t
    {
        LEFT = 1,
        MIDDLE = 2,
        RIGHT = 4
    };

    struct MousePosition
    {
        bool MouseMoved;
        float X = 0, Y = 0;
        float DeltaX = 0, DeltaY = 0;
    };

    // ActiveWindow - window which callback functions should be called when window-related events occur 
    void ReadEvents(platform::Window* ActiveWindow);

    bool IsKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyPressed(const SDL_Keycode& KeyCode);
    bool WasKeyReleased(const SDL_Keycode& KeyCode);

    bool IsMouseButtonPressed(const MouseButton& Button);
    bool WasMouseButtonPressed(const MouseButton& Button);
    bool WasMouseButtonReleased(const MouseButton& Button);

    MousePosition GetMousePostion();
    float GetMouseWheelDelta();
} // namespace lucid