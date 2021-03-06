#pragma once

#include "devices/gpu/gpu.hpp"

namespace lucid::platform
{
    struct WindowDefiniton
    {
        char const* title;
        u16 x, y;
        u16 width, height;
        bool sRGBFramebuffer;
    };

    class Window
    {
      public:
        Window() = default;

        virtual void Swap() = 0;
        virtual void Prepare() = 0;

        virtual u16 GetWidth() const = 0;
        virtual u16 GetHeight() const = 0;
        virtual float GetAspectRatio() const = 0;
        virtual gpu::Framebuffer* GetFramebuffer() const = 0;
        
        virtual void Show() = 0;
        virtual void Hide() = 0;

        virtual void Destroy() = 0;

        // Events callbacks
        
        virtual void OnFocusGained() = 0;

        virtual ~Window() = default;
    };

    Window* CreateWindow(const WindowDefiniton& Definiton);

} // namespace lucid::platform