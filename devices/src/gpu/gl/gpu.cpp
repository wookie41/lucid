#include "devices/gpu/gl/gpu.hpp"
#include "devices/gpu/buffer.hpp"

#include "SDL2/SDL.h"

#include <cassert>

#ifndef NDEBUG
#include <stdio.h>
#endif

static const GLenum GL_TYPES[] = { GL_BYTE,
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
                                   0 };

GLenum GL_DRAW_MODES[] = { GL_POINTS,
                           GL_LINE_STRIP,
                           GL_LINE_LOOP,
                           GL_LINES,
                           GL_LINE_STRIP_ADJACENCY,
                           GL_LINES_ADJACENCY,
                           GL_TRIANGLE_STRIP,
                           GL_TRIANGLE_FAN,
                           GL_TRIANGLES,
                           GL_TRIANGLE_STRIP_ADJACENCY,
                           GL_TRIANGLES_ADJACENCY,
                           GL_PATCHES };

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
        glVertexAttribPointer(Attribute.Index, Attribute.Size, GL_TYPES[Attribute.Type], GL_FALSE,
                              Attribute.Stride, (void*)Attribute.Offset);

        glEnableVertexAttribArray(Attribute.Index);
        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
    }

    void AddIntegerVertexAttribute(const VertexAttribute& Attribute)
    {
        glVertexAttribIPointer(Attribute.Index, Attribute.Size, GL_TYPES[Attribute.Type],
                               Attribute.Stride, (void*)Attribute.Offset);

        glEnableVertexAttribArray(Attribute.Index);
        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
    }

    void AddLongVertexAttribute(const VertexAttribute& Attribute)
    {
        glVertexAttribLPointer(Attribute.Index, Attribute.Size, GL_TYPES[Attribute.Type],
                               Attribute.Stride, (void*)Attribute.Offset);

        glEnableVertexAttribArray(Attribute.Index);
        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
    }

    GLenum toGLDataType(const Type& type)
    {
        GLenum glType = GL_TYPES[type];
        assert(glType);
        return glType;
    }

    VertexArray* CreateVertexArray(const StaticArray<VertexAttribute> const* VertexArrayAttributes,
                                   const Buffer const* VertexBuffer,
                                   const Buffer const* ElementBuffer)
    {
        GLuint VAO;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        VertexBuffer->Bind(BufferBindPoint::VERTEX);

        if (ElementBuffer != nullptr)
            ElementBuffer->Bind(BufferBindPoint::ELEMENT);

        for (uint32_t attrIdx = 0; attrIdx < VertexArrayAttributes->Length; ++attrIdx)
        {
            VertexAttribute* vertexAttribute = VertexArrayAttributes[attrIdx];

            switch (vertexAttribute->Type)
            {
            case lucid::Type::FLOAT:
                AddVertexAttribute(*vertexAttribute);
                break;
            case lucid::Type::BYTE:
            case lucid::Type::UINT_8:
            case lucid::Type::UINT_16:
            case lucid::Type::UINT_32:
                AddIntegerVertexAttribute(*vertexAttribute);
                break;
            case lucid::Type::DOUBLE:
                AddLongVertexAttribute(*vertexAttribute);
                break;
            default:
#ifndef NDEBUG
                printf("Unsupported vertex attribute used (%d)\n", vertexAttribute->Type);
#endif
                break;
            }
        }

        glBindVertexArray(0);

        return new GLVertexArray(VAO);
    }

    GLVertexArray::GLVertexArray(const GLuint& GLVAOHandle) : glVAOHandle(GLVAOHandle) {}

    void GLVertexArray::Bind() { glBindVertexArray(glVAOHandle); }

    void GLVertexArray::Unbind() { glBindVertexArray(0); }

    void GLVertexArray::EnableAttribute(const uint32_t& AttributeIndex)
    {
        glEnableVertexArrayAttrib(glVAOHandle, AttributeIndex);
    }

    void GLVertexArray::DisableAttribute(const uint32_t& AttributeIndex)
    {
        glDisableVertexArrayAttrib(glVAOHandle, AttributeIndex);
    }

    GLVertexArray::~GLVertexArray() { glDeleteVertexArrays(1, &glVAOHandle); }

    void DrawVertices(const uint32_t& First, const uint32_t& Count, const DrawMode& Mode)
    {
        glDrawArrays(GL_DRAW_MODES[Mode], First, Count);
    }

    void DrawElement(const Type& IndiciesType, const uint32_t& Count, const DrawMode& Mode)
    {
        glDrawElements(GL_DRAW_MODES[Mode], Count, GL_UNSIGNED_INT, 0);
    }
} // namespace lucid::gpu