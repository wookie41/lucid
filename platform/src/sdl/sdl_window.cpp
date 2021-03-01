#include "platform/sdl/sdl_window.hpp"
#include "SDL2/SDL.h"
#include "GL/glew.h"
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
        SDL_GLContext context = SDL_GL_CreateContext(window);

        SDL_CaptureMouse(SDL_TRUE);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        return new SDLWindow(window, context, Definition.width, Definition.height);
    }

    SDLWindow::SDLWindow(SDL_Window* Window, SDL_GLContext Context, const u16& Width, const u16& Height)
    : window(Window), context(Context), width(Width), height(Height), aspectRatio((float)Width / (float)Height)
    {
    }

    void SDLWindow::Swap() { SDL_GL_SwapWindow(window); }

    void SDLWindow::Prepare() { SDL_GL_MakeCurrent(window, context); }

    u16 SDLWindow::GetWidth() const { return width; }

    u16 SDLWindow::GetHeight() const { return height; }

    float SDLWindow::GetAspectRatio() const { return aspectRatio; }

    void SDLWindow::Show() { SDL_ShowWindow(window); }

    void SDLWindow::Hide() { SDL_HideWindow(window); }

    void SDLWindow::Destroy()
    {
        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);
        SDL_Quit();
    }

    void SDLWindow::OnFocusGained()
    {
        SDL_CaptureMouse(SDL_TRUE);
    }

} // namespace lucid::platform