#include "devices/gpu/gpu.hpp"

#include "SDL2/SDL.h"
#include "GL/glew.h"

#include <cassert>

static const GLenum GL_TYPES[] = {
        GL_BYTE,
        GL_SHORT,
        GL_INT,
        0,
        
        GL_UNSIGNED_BYTE,
        GL_UNSIGNED_SHORT,
        GL_UNSIGNED_INT,
        0,

        GL_FLOAT,
        GL_DOUBLE,
        
        0,
        0,
        0,
        
        0,
        0,
        0,

        0,
        0
};

namespace lucid::gpu
{
    void Init()
    {
        int SDLInitResult = SDL_Init(SDL_INIT_VIDEO);
        assert(SDLInitResult == 0);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // NOTE later on, this could be loaded from some kind of a ini file or something
        // so we can control which version of OpenGL is the engine using
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_Window* window = SDL_CreateWindow("xxx", 1, 1, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_GLContext context = SDL_GL_CreateContext(window);

        GLenum GLEWInitResult = glewInit();
        assert(GLEWInitResult == GLEW_OK);

        SDL_DestroyWindow(window);
        SDL_GL_DeleteContext(context);
    }

    void AddVertexAttribute(const VertexAttribute& Attribute)
    {
        glVertexAttribPointer(Attribute.index, Attribute.size, GL_TYPES[Attribute.type],
                              Attribute.normalized, Attribute.stride, (void*)Attribute.offset);

        glEnableVertexAttribArray(Attribute.index);
        glVertexAttribDivisor(Attribute.index, Attribute.divisor);
    }

    void AddIntegerVertexAttribute(const VertexAttribute& Attribute)
    {
        glVertexAttribIPointer(Attribute.index, Attribute.size, GL_TYPES[Attribute.type],
                               Attribute.stride, (void*)Attribute.offset);

        glEnableVertexAttribArray(Attribute.index);
        glVertexAttribDivisor(Attribute.index, Attribute.divisor);
    }

    void AddLongVertexAttribute(const VertexAttribute& Attribute)
    {
        glVertexAttribLPointer(Attribute.index, Attribute.size, GL_TYPES[Attribute.type],
                               Attribute.stride, (void*)Attribute.offset);

        glEnableVertexAttribArray(Attribute.index);
        glVertexAttribDivisor(Attribute.index, Attribute.divisor);
    }

    GLenum toGLDataType(const Type& type)
    {
        GLenum glType = GL_TYPES[type];
        assert(glType);
        return glType;
    }
} // namespace lucid::gpu