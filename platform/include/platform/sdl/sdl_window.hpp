#pragma once

#include "devices/gpu/framebuffer.hpp"
#include "platform/window.hpp"

struct SDL_Window;
typedef void* SDL_GLContext;

namespace lucid::platform
{
    class SDLWindow : public Window
    {
      public:
        SDLWindow(SDL_Window* InSDLWindow, SDL_GLContext InContext, const u16& InWidth, const u16& InHeight);

        virtual void Swap() override;
        virtual void Prepare() override;

        virtual u16 GetWidth() const override;
        virtual u16 GetHeight() const override;
        virtual float GetAspectRatio() const override;
        virtual gpu::Framebuffer* GetFramebuffer() const override;

        virtual void Show() override;
        virtual void Hide() override;

        virtual void OnFocusGained() override;

        virtual void Destroy() override;

        virtual ~SDLWindow() = default;

      private:

        float AspectRatio;
        u16 Width, Height;
        SDL_Window* MySDLWindow;
        SDL_GLContext MyGLContext;
        gpu::Framebuffer* WindowFramebuffer;
    };
}; // namespace lucid::platform