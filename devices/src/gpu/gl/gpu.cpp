#include "devices/gpu/gpu.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    // Buffers functions
    void ClearBuffers(const ClearableBuffers& BuffersToClear)
    {
        static GLbitfield buffersBits[] = { GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT,
                                            GL_ACCUM_BUFFER_BIT, GL_STENCIL_BUFFER_BIT };

        GLbitfield glBuffersBitField = 0;
        if (BuffersToClear & ClearableBuffers::COLOR)
        {
            glBuffersBitField |= GL_COLOR_BUFFER_BIT;
        }

        if (BuffersToClear & ClearableBuffers::DEPTH)
        {
            glBuffersBitField |= GL_DEPTH_BUFFER_BIT;
        }

        if (BuffersToClear & ClearableBuffers::STENCIL)
        {
            glBuffersBitField |= GL_COLOR_BUFFER_BIT;
        }

        if (BuffersToClear & ClearableBuffers::ACCUMULATION)
        {
            glBuffersBitField |= GL_ACCUM_BUFFER_BIT;
        }

        glClear(glBuffersBitField);
    }

    void SetClearColor(const color& Color) { glClearColor(Color.r, Color.g, Color.b, Color.a); }

    // Depth test functions

    void EnableDepthTest() { glEnable(GL_DEPTH_TEST); }

    void DisableDepthTest() { glDisable(GL_DEPTH_TEST); }

    void SetViewprot(const Viewport& ViewportToUse)
    {
        glViewport(ViewportToUse.X, ViewportToUse.Y, ViewportToUse.Width, ViewportToUse.Height);
    }
} // namespace lucid::gpu
