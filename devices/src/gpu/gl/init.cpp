#include "devices/gpu/init.hpp"

#include "SDL2/SDL.h"
#include "GL/glew.h"
#include <cassert>

namespace lucid::gpu
{
    int Init(const GPUSettings& Setings)
    {
        int SDLInitResult = SDL_Init(SDL_INIT_VIDEO);
        if (SDLInitResult != 0)
        {
            // @TODO LOG
            return -1;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // @TODO later on, this could be loaded from some kind of a ini file or something
        // so we can control the minmum version of OpenGL requried
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_Window* window = SDL_CreateWindow("xxx", 1, 1, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
        if (window == nullptr)
        {
            // @TODO Log
            return -1;
        }

        SDL_GLContext context = SDL_GL_CreateContext(window);

        GLenum GLEWInitResult = glewInit();
        if (GLEWInitResult != GLEW_OK)
        {
            //@TODO Log
            return -1;
        }

        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);

        //@TODO Log
        return 0;
    }
} // namespace devices::gpu
