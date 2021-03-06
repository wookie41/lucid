#include "platform/sdl/sdl_window.hpp"

#include "common/log.hpp"
#include "devices/gpu/gl/framebuffer.hpp"
#include "SDL2/SDL.h"

namespace lucid::platform
{
    Window* CreateWindow(const WindowDefiniton& Definition)
    {
        if (Definition.sRGBFramebuffer)
        {
            SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
        }
        SDL_Window* window = SDL_CreateWindow(Definition.title, Definition.x, Definition.y, Definition.width, Definition.height,
                                              SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
        if (window == nullptr)
        {
            LUCID_LOG(LogLevel::ERR, "[SDL] Failed to create a new window: %s", SDL_GetError());
            return nullptr;
        }
        SDL_GLContext context = SDL_GL_CreateContext(window);
        if (context == nullptr)
        {
            SDL_DestroyWindow(window);
            LUCID_LOG(LogLevel::ERR, "[SDL] Failed to create a context for the window: %s", SDL_GetError());
            return nullptr;
        }

        SDL_GL_MakeCurrent(window, context);
        SDL_CaptureMouse(SDL_TRUE);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        return new SDLWindow(window, context, Definition.width, Definition.height);
    }

    SDLWindow::SDLWindow(SDL_Window* InWindow, SDL_GLContext InContext, const u16& InWidth, const u16& InHeight)
    : MySDLWindow(InWindow), MyGLContext(InContext), Width(InWidth), Height(InHeight), AspectRatio((float)InWidth / (float)InHeight)
    {
        WindowFramebuffer = new gpu::GLDefaultFramebuffer{ InWidth, InHeight };
    }

    void SDLWindow::Swap() { SDL_GL_SwapWindow(MySDLWindow); }

    void SDLWindow::Prepare() { SDL_GL_MakeCurrent(MySDLWindow, MyGLContext); }

    u16 SDLWindow::GetWidth() const { return Width; }

    u16 SDLWindow::GetHeight() const { return Height; }

    float SDLWindow::GetAspectRatio() const { return AspectRatio; }

    gpu::Framebuffer* SDLWindow::GetFramebuffer() const
    {
        return WindowFramebuffer;
    }

    void SDLWindow::Show() { SDL_ShowWindow(MySDLWindow); }

    void SDLWindow::Hide() { SDL_HideWindow(MySDLWindow); }

    void SDLWindow::Destroy()
    {
        delete WindowFramebuffer;
        SDL_GL_MakeCurrent(MySDLWindow, MyGLContext);
        SDL_GL_DeleteContext(MyGLContext);
        SDL_DestroyWindow(MySDLWindow);
    }

    void SDLWindow::OnFocusGained()
    {
        SDL_CaptureMouse(SDL_TRUE);
    }

} // namespace lucid::platform