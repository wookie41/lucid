#include "devices/gpu/init.hpp"

#include "SDL2/SDL.h"
#include "GL/glew.h"
#include <cassert>

#include "common/log.hpp"

namespace lucid::gpu
{
    int Init(const GPUSettings& Setings)
    {
        LUCID_LOG(LogLevel::INFO, "Initializing GPU...");

        int SDLInitResult = SDL_Init(SDL_INIT_VIDEO);
        if (SDLInitResult != 0)
        {
            LUCID_LOG(LogLevel::ERROR, "Failed to initialize SDL_VIDEO");
            return -1;
        }

        LUCID_LOG(LogLevel::INFO, "Initialized SDL_VIDEO");

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
            LUCID_LOG(LogLevel::ERROR, "Failed to create dummy window");
            return -1;
        }

        SDL_GLContext context = SDL_GL_CreateContext(window);

        GLenum GLEWInitResult = glewInit();
        if (GLEWInitResult != GLEW_OK)
        {

            LUCID_LOG(LogLevel::INFO, (char*)glewGetErrorString(GLEWInitResult));
            return -1;
        }

        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);

        LUCID_LOG(LogLevel::INFO, "GPU initialized");

        return 0;
    }
} // namespace devices::gpu
