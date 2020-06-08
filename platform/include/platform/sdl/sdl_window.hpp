#pragma once

#include "platform/window.hpp"

struct SDL_Window;
typedef void* SDL_GLContext;

namespace lucid::platform
{
    class SDLWindow : public Window
    {
      public:
        SDLWindow(SDL_Window* Window, SDL_GLContext Context);

        virtual void Swap() override;
        virtual void Prepare() override;

        void SetClearColor(const math::vec4& color) override;
        void ClearColor() override;

        virtual void Show() override;
        virtual void Hide() override;

        virtual void Destroy() override;

        virtual ~SDLWindow() = default;

      private:
        SDL_Window* window;
        SDL_GLContext context;
    };
}; // namespace lucid::platform