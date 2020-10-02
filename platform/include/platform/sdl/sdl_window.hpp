#pragma once

#include "platform/window.hpp"

struct SDL_Window;
typedef void* SDL_GLContext;

namespace lucid::platform
{
    class SDLWindow : public Window
    {
      public:
        SDLWindow(SDL_Window* Window, SDL_GLContext Context, const uint16_t& Width, const uint16_t& Height);

        virtual void Swap() override;
        virtual void Prepare() override;

        virtual uint16_t GetWidth() const override;
        virtual uint16_t GetHeight() const override;
      
        virtual void Show() override;
        virtual void Hide() override;

        virtual void Destroy() override;

        virtual ~SDLWindow() = default;

      private:
        uint16_t width, height;
        SDL_Window* window;
        SDL_GLContext context;
    };
}; // namespace lucid::platform