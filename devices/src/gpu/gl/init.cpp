#include "devices/gpu/init.hpp"
#include "devices/gpu/gpu.hpp"

#include "SDL2/SDL.h"
#include "GL/glew.h"

#include <cassert>

#include "common/log.hpp"

namespace lucid::gpu
{
    GPUInfo Info;

    void InitGPUInfo();

    int Init(const GPUSettings& Setings)
    {
        LUCID_LOG(LogLevel::INFO, "Initializing GPU...");

        int SDLInitResult = SDL_Init(SDL_INIT_VIDEO);
        if (SDLInitResult != 0)
        {
            LUCID_LOG(LogLevel::ERR, "Failed to initialize SDL_VIDEO");
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

        SDL_Window* window = SDL_CreateWindow("xxx", 200, 200, 200, 200, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
        if (window == nullptr)
        {
            LUCID_LOG(LogLevel::ERR, "[SDL] Failed to create dummy window: %s", SDL_GetError());
            return -1;
        }

        SDL_GLContext context = SDL_GL_CreateContext(window);
        if (context == nullptr)
        {
            LUCID_LOG(LogLevel::ERR, "[SDL] Failed to create dummy context: %s", SDL_GetError());
            return -1;
        }


        SDL_GL_MakeCurrent(window, context);

        GLenum GLEWInitResult = glewInit();
        if (GLEWInitResult != GLEW_OK)
        {
            LUCID_LOG(LogLevel::INFO, (char*)glewGetErrorString(GLEWInitResult));
            return -1;
        }

        InitGPUInfo();
        
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);

        LUCID_LOG(LogLevel::INFO, "GPU initialized");

        return 0;
    }

    void InitGPUInfo()
    {
        GLint property;

        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &property);
        LUCID_LOG(LogLevel::INFO, "Available texture units = %d", property);
        Info.BoundTextures = new Texture*[property];
        Info.MaxColorAttachments = property;

        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &property);
        LUCID_LOG(LogLevel::INFO, "Available draw buffers units = %d", property);
        Info.MaxColorAttachments = property;

        for (int i = 0; i < property; ++i)
        {
            Info.BoundTextures[i] = nullptr;
        }
    }

    void Shutdown()
    {
        SDL_Quit();
    }

} // namespace lucid::gpu
