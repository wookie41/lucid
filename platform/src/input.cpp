#include "platform/input.hpp"
#include "SDL2/SDL_events.h"
#include "common/bytes.hpp"

namespace lucid
{

    struct KeyboardState
    {
        uint64_t pressedKeys[4] = { 0 }; // keeps track of which keys are currently held down
        uint64_t clickedKeys[4] = { 0 }; // cleared per frame, only tells which keys were pressed down this frame
        uint64_t releasedKeys[4] = { 0 }; // cleared per frame, only tells which keys were released this frame
    };

    struct MouseState
    {
        MousePosition Position;
        float WheelDelta = 0;
        uint8_t PressedButtons = 0;
        uint8_t ClickedButtons = 0;
        uint8_t ReleasedButtons = 0;
    };

    const SDL_Keycode MIN_SPECIAL_KEY_KEYCODE = SDLK_CAPSLOCK;

    inline uint8_t calculateKeyBit(const SDL_Keycode& code)
    {
        return code < MIN_SPECIAL_KEY_KEYCODE ? code : ((MIN_SPECIAL_KEY_KEYCODE & 0x8) + 128);
    }

    inline void setHigh(const SDL_Keycode& keyCode, uint64_t* keyMap)
    {
        uint8_t keyBit = calculateKeyBit(keyCode);
        uint8_t mapIdx = keyBit / 64;
        keyBit %= 64;
        keyMap[mapIdx] |= ((uint64_t)1 << keyBit);
    }

    inline void setLow(const SDL_Keycode& keyCode, uint64_t* keyMap)
    {
        uint8_t keyBit = calculateKeyBit(keyCode);
        uint8_t mapIdx = keyBit / 64;
        keyBit %= 64;
        keyMap[mapIdx] &= ~((uint64_t)1 << keyBit);
    }

    static KeyboardState keyboardState;
    static MouseState mouseState;

    void ReadEvents()
    {
        for (uint8_t i = 0; i < 4; ++i)
            keyboardState.clickedKeys[i] = 0;

        for (uint8_t i = 0; i < 4; ++i)
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
            switch (event.type)
            {
            case SDL_KEYDOWN:
                setHigh(event.key.keysym.sym, keyboardState.clickedKeys);
                setHigh(event.key.keysym.sym, keyboardState.pressedKeys);
                break;

            case SDL_KEYUP:
                setHigh(event.key.keysym.sym, keyboardState.releasedKeys);
                setLow(event.key.keysym.sym, keyboardState.pressedKeys);
                break;

            case SDL_MOUSEBUTTONDOWN:
                mouseState.PressedButtons |=
                (mouseState.ClickedButtons |= (1 << (event.button.button - 1)));
                break;

            case SDL_MOUSEBUTTONUP:
                mouseState.PressedButtons &=
                ~(mouseState.ReleasedButtons |= (1 << (event.button.button - 1)));
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

    inline bool isHigh(const SDL_Keycode& keyCode, uint64_t* keyMap)
    {
        uint8_t keyBit = calculateKeyBit(keyCode);
        uint8_t mapIdx = keyBit / 64;
        keyBit %= 64;
        return keyMap[mapIdx] & ((uint64_t)1 << keyBit);
    }

    bool IsKeyPressed(const SDL_Keycode& KeyCode)
    {
        return isHigh(KeyCode, keyboardState.pressedKeys);
    }

    bool WasKeyPressed(const SDL_Keycode& KeyCode)
    {
        return isHigh(KeyCode, keyboardState.clickedKeys);
    }

    bool WasKeyReleased(const SDL_Keycode& KeyCode)
    {
        return isHigh(KeyCode, keyboardState.releasedKeys);
    }

    bool IsMouseButtonPressed(const MouseButton& Button)
    {
        return mouseState.PressedButtons & Button;
    }

    bool WasMouseButtonPressed(const MouseButton& Button)
    {
        return mouseState.ClickedButtons & Button;
    }

    bool WasMouseButtonReleased(const MouseButton& Button)
    {
        return mouseState.ReleasedButtons & Button;
    }

    MousePosition GetMousePostion() { return mouseState.Position; }
    float GetMouseWheelDelta() { return mouseState.WheelDelta; }
} // namespace lucid