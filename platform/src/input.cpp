#include "platform/input.hpp"
#include "SDL2/SDL_events.h"
#include "common/bytes.hpp"

namespace lucid
{
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
        keyMap[mapIdx] |= (1 << keyBit);
    }

    inline void setLow(const SDL_Keycode& keyCode, uint64_t* keyMap)
    {
        uint8_t keyBit = calculateKeyBit(keyCode);
        uint8_t mapIdx = keyBit / 64;
        keyBit %= 64;
        keyMap[mapIdx] &= ~(1 << keyBit);
    }

    static KeyboardState keyboardState;

    void ReadEvents()
    {
        zero(keyboardState.clickedKeys, sizeof(keyboardState.clickedKeys));
        zero(keyboardState.releasedKeys, sizeof(keyboardState.releasedKeys));

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
            }
        }
    }

    inline bool isHigh(const SDL_Keycode& keyCode, uint64_t* keyMap)
    {
        uint8_t keyBit = calculateKeyBit(keyCode);
        uint8_t mapIdx = keyBit / 64;
        keyBit %= 64;
        return keyMap[mapIdx] & (1 << keyBit);
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
} // namespace lucid