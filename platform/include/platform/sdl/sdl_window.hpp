#pragma once

#include "platform/window.hpp"

struct SDL_Window;
typedef void* SDL_GLContext;

namespace lucid::platform
{
    class SDLWindow : public Window
    {
      public:
        SDLWindow(SDL_Window* Window, SDL_GLContext Context, const u16& Width, const u16& Height);

        virtual void Swap() override;
        virtual void Prepare() override;

        virtual u16 GetWidth() const override;
        virtual u16 GetHeight() const override;
        virtual float GetAspectRatio() const override;
      
        virtual void Show() override;
        virtual void Hide() override;

        virtual void OnFocusGained() override;

        virtual void Destroy() override;

        virtual ~SDLWindow() = default;

      private:
        float aspectRatio;
        u16 width, height;
        SDL_Window* window;
        SDL_GLContext context;
    };
}; // namespace lucid::platform