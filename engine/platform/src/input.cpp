#include "platform/input.hpp"

#include <imgui_impl_sdl.h>

#include "SDL2/SDL_events.h"
#include "common/bytes.hpp"
#include "platform/window.hpp"

namespace lucid
{

    struct FKeyboardState
    {
        u64 pressedKeys[4] = { 0 }; // keeps track of which keys are currently held down
        u64 clickedKeys[4] = { 0 }; // cleared per frame, only tells which keys were pressed down this frame
        u64 releasedKeys[4] = { 0 }; // cleared per frame, only tells which keys were released this frame
    };

    struct FMouseState
    {
        FMousePosition Position;
        float WheelDelta = 0;
        u8 PressedButtons = 0;
        u8 ClickedButtons = 0;
        u8 ReleasedButtons = 0;
    };

    const SDL_Keycode MIN_SPECIAL_KEY_KEYCODE = SDLK_CAPSLOCK;

    inline u8 calculateKeyBit(const SDL_Keycode& code)
    {
        return code < MIN_SPECIAL_KEY_KEYCODE ? code : ((MIN_SPECIAL_KEY_KEYCODE & 0x8) + 128);
    }

    inline void setHigh(const SDL_Keycode& keyCode, u64* keyMap)
    {
        u8 keyBit = calculateKeyBit(keyCode);
        u8 mapIdx = keyBit / 64;
        keyBit %= 64;
        keyMap[mapIdx] |= ((u64)1 << keyBit);
    }

    inline void setLow(const SDL_Keycode& keyCode, u64* keyMap)
    {
        u8 keyBit = calculateKeyBit(keyCode);
        u8 mapIdx = keyBit / 64;
        keyBit %= 64;
        keyMap[mapIdx] &= ~((u64)1 << keyBit);
    }

    static FKeyboardState keyboardState;
    static FMouseState mouseState;

    void ReadEvents(platform::CWindow* ActiveWindow)
    {
        for (u8 i = 0; i < 4; ++i)
            keyboardState.clickedKeys[i] = 0;

        for (u8 i = 0; i < 4; ++i)
            keyboardState.releasedKeys[i] = 0;

        mouseState.ClickedButtons = 0;
        mouseState.ReleasedButtons = 0;
        mouseState.Position.DeltaX = 0;
        mouseState.Position.DeltaY = 0;
        mouseState.WheelDelta = 0;
        mouseState.Position.MouseMoved = false;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
#if DEVELOPMENT
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif
            /* Window events */

            if (event.type == SDL_WINDOWEVENT)
            {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    ActiveWindow->OnFocusGained();
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    ActiveWindow->OnResize(event.window.data1, event.window.data2);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    ActiveWindow->OnRequestClose();
                    break;
                }

                continue;
            }

            switch (event.type)
            {

                /* Keyboard events */

            case SDL_KEYDOWN:
                setHigh(event.key.keysym.sym, keyboardState.clickedKeys);
                setHigh(event.key.keysym.sym, keyboardState.pressedKeys);
                break;

            case SDL_KEYUP:
                setHigh(event.key.keysym.sym, keyboardState.releasedKeys);
                setLow(event.key.keysym.sym, keyboardState.pressedKeys);
                break;

                /* Mouse Events */

            case SDL_MOUSEBUTTONDOWN:
                mouseState.PressedButtons |= (mouseState.ClickedButtons |= (1 << (event.button.button - 1)));
                break;

            case SDL_MOUSEBUTTONUP:
                mouseState.PressedButtons &= ~(mouseState.ReleasedButtons |= (1 << (event.button.button - 1)));
                break;

            case SDL_MOUSEMOTION:
                mouseState.Position.MouseMoved = true;
                mouseState.Position.DeltaX = mouseState.Position.X - event.motion.x;
                mouseState.Position.DeltaY = mouseState.Position.Y - event.motion.y;
                mouseState.Position.X = event.motion.x;
                mouseState.Position.Y = event.motion.y;
                break;

            case SDL_MOUSEWHEEL:
                mouseState.WheelDelta = event.wheel.y;
                break;
            }
        }
    }

    inline bool isHigh(const SDL_Keycode& keyCode, u64* keyMap)
    {
        uint8_t keyBit = calculateKeyBit(keyCode);
        uint8_t mapIdx = keyBit / 64;
        keyBit %= 64;
        return keyMap[mapIdx] & ((u64)1 << keyBit);
    }

    bool IsKeyPressed(const SDL_Keycode& KeyCode) { return isHigh(KeyCode, keyboardState.pressedKeys); }

    bool WasKeyPressed(const SDL_Keycode& KeyCode) { return isHigh(KeyCode, keyboardState.clickedKeys); }

    bool WasKeyReleased(const SDL_Keycode& KeyCode) { return isHigh(KeyCode, keyboardState.releasedKeys); }

    bool IsMouseButtonPressed(const EMouseButton& Button) { return mouseState.PressedButtons & Button; }

    bool WasMouseButtonPressed(const EMouseButton& Button) { return mouseState.ClickedButtons & Button; }

    bool WasMouseButtonReleased(const EMouseButton& Button) { return mouseState.ReleasedButtons & Button; }

    FMousePosition GetMousePostion() { return mouseState.Position; }

    float GetMouseWheelDelta() { return mouseState.WheelDelta; }
} // namespace lucid