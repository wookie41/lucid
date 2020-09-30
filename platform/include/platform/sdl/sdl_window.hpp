#pragma once

#include "platform/window.hpp"

struct SDL_Window;
typedef void* SDL_GLContext;

namespace lucid::platform
{
    class SDLWindow : public Window
    {
      public:
        SDLWindow(SDL_Window* Window, SDL_GLContext Context, const math::ivec2& Size);

        virtual void Swap() override;
        virtual void Prepare() override;

        virtual math::ivec2 GetSize() const override;
      
        virtual void Show() override;
        virtual void Hide() override;

        virtual void Destroy() override;

        virtual ~SDLWindow() = default;

      private:
        math::ivec2 size;
        SDL_Window* window;
        SDL_GLContext context;
    };
}; // namespace lucid::platform