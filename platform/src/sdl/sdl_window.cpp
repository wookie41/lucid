#include "platform/include/sdl/sdl_window.hpp"
#include "SDL2/SDL.h"

namespace lucid::platform
{
    Window* CreateWindow(const WindowDefiniton& Definition)
    {
        SDL_Window* window = SDL_CreateWindow(
            Definition.title, Definition.x, Definition.y, Definition.width,
            Definition.height, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
        );
        
        SDL_GLContext context = SDL_GL_CreateContext(window);

        return new SDLWindow(window, context);
    }

    SDLWindow::SDLWindow(SDL_Window* Window, SDL_GLContext Context)
    {
        window = Window;
        context = Context;
    }

    void SDLWindow::Swap()
    {
        SDL_GL_SwapWindow(window);
    }

    void SDLWindow::Prepare()
    {
        SDL_GL_MakeCurrent(window, context);
    }

    void SDLWindow::Show()
    {
        SDL_ShowWindow(window);
    }
   
    void SDLWindow::Hide()
    {
        SDL_HideWindow(window);
    }

    void SDLWindow::Destroy()
    {
        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);

    }
} // namespace lucid::platform