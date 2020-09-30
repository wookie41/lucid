#pragma once

#include "common/math.hpp"
#include "devices/gpu/gpu.hpp"

namespace lucid::platform
{
    struct WindowDefiniton
    {
        char const* title;
        uint16_t x, y;
        uint16_t width, height;
    };

    class Window
    {
      public:
        Window() = default;

        virtual void Swap() = 0;
        virtual void Prepare() = 0;

        virtual math::ivec2 GetSize() const = 0;

        virtual void Show() = 0;
        virtual void Hide() = 0;

        virtual void Destroy() = 0;

        virtual ~Window() = default;
    };

    Window* CreateWindow(const WindowDefiniton& Definiton);

} // namespace lucid::platform