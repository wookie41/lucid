#pragma once

#include "devices/gpu/gpu.hpp"

namespace lucid::platform
{
    struct WindowDefiniton
    {
        char const* title;
        uint16_t x, y;
        uint16_t width, height;
        bool sRGBFramebuffer;
    };

    class Window
    {
      public:
        Window() = default;

        virtual void Swap() = 0;
        virtual void Prepare() = 0;

        virtual uint16_t GetWidth() const = 0;
        virtual uint16_t GetHeight() const = 0;
        virtual float GetAspectRatio() const = 0;
        
        virtual void Show() = 0;
        virtual void Hide() = 0;

        virtual void Destroy() = 0;

        virtual ~Window() = default;
    };

    Window* CreateWindow(const WindowDefiniton& Definiton);

} // namespace lucid::platform