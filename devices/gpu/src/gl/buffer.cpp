#include "devices/gpu/include/gl/buffer.hpp"

static const GLenum GL_BIND_POINTS[] = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER };
static const GLenum GL_USAGE_HINTS[] = { GL_STATIC_DRAW, GL_DYNAMIC_DRAW };

namespace lucid::gpu
{
    GLBuffer::GLBuffer(const GLuint& BufferHandle) : glBufferHandle(BufferHandle) {}
    
    void GLBuffer::Bind(const BufferBindPoint& BindPoint)
    {
        glBindBuffer(GL_BIND_POINTS[BindPoint], glBufferHandle);
    }

    void GLBuffer::BindIndexed(const uint32_t& index, const BufferBindPoint& BindPoint)
    {
        glBindBufferBase(GL_BIND_POINTS[BindPoint], index, glBufferHandle);
    }

    void GLBuffer::Download(void* Destination)
    {
        // TODO
    }

    void* GLBuffer::MemoryMap()
    {
        // TODO
    }

    void GLBuffer::Free() { glDeleteBuffers(1, &glBufferHandle); }

    Buffer* CreateBuffer(const BufferData& BufferData)
    {
        GLuint bufferHandle;

        glGenBuffers(1, &bufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, bufferHandle);
        glBufferData(GL_COPY_WRITE_BUFFER, BufferData.size, BufferData.data,
                     GL_USAGE_HINTS[BufferData.usage]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        return new GLBuffer(bufferHandle);
    };

} // namespace lucid::gpu