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
        math::ivec2 position = { 0, 0 };
        math::ivec2 positionDelta = { 0, 0 };
        float wheelDelta = 0;
        uint8_t pressedButtons = 0;
        uint8_t clickedButtons = 0;
        uint8_t releasedButtons = 0;
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

        mouseState.clickedButtons = mouseState.releasedButtons = 0;

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
                mouseState.pressedButtons |=
                (mouseState.clickedButtons |= (1 << (event.button.button - 1)));
                break;

            case SDL_MOUSEBUTTONUP:
                mouseState.pressedButtons &=
                ~(mouseState.releasedButtons |= (1 << (event.button.button - 1)));
                break;

            case SDL_MOUSEMOTION:
                mouseState.positionDelta = { mouseState.position.x - event.motion.x,
                                             mouseState.position.y - event.motion.y };
                mouseState.position = { event.motion.x, event.motion.y };
                break;

            case SDL_MOUSEWHEEL:
                mouseState.wheelDelta = event.wheel.y;
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
        return mouseState.pressedButtons & Button;
    }

    bool WasMouseButtonPressed(const MouseButton& Button)
    {
        return mouseState.clickedButtons & Button;
    }

    bool WasMouseButtonReleased(const MouseButton& Button)
    {
        return mouseState.releasedButtons & Button;
    }

    math::ivec2 GetMousePostion() { return mouseState.position; }
    math::ivec2 GetMousePostionDelta() { return mouseState.positionDelta; }
    float GetMouseWheelDelta() { return mouseState.wheelDelta; }
} // namespace lucid