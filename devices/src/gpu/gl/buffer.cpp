#include "devices/gpu/gl/buffer.hpp"
#include <cassert>

#define NO_COHERENT_OR_PERSISTENT_BIT_SET(FLAGS) \
    (((FLAGS & (uint16_t)BufferAccessPolicy::COHERENT) | (FLAGS & (uint16_t)BufferAccessPolicy::PERSISTENT)) == 0)

#define AS_GL_BIND_POINT(bindPoint) (GL_BIND_POINTS[((uint16_t)bindPoint) - 1])

static const GLenum GL_BIND_POINTS[] = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER };

static const GLenum GL_MUTABLE_USAGE_HINTS[] = { GL_STATIC_DRAW, GL_DYNAMIC_DRAW,
                                                 GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER };

static const GLenum GL_IMMUTABLE_ACCESS_BITS[] = { GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT,
                                                   GL_MAP_WRITE_BIT,       GL_MAP_PERSISTENT_BIT,
                                                   GL_MAP_COHERENT_BIT,    GL_CLIENT_STORAGE_BIT };

static const GLenum GL_MAP_ACCESS_BITS[] = { GL_MAP_READ_BIT, GL_MAP_WRITE_BIT,
                                             GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT };

namespace lucid::gpu
{
    GLbitfield calculateGLMapAccessBits(const uint16_t& MapPolicy)
    {
        uint8_t accessBit = 1;
        GLbitfield glAccessBits = 0;

        for (uint8_t accessBitShift = 0; accessBitShift < sizeof(GL_MAP_ACCESS_BITS) / sizeof(GLenum); ++accessBitShift)
        {
            if (MapPolicy & (accessBit << accessBitShift))
            {
                glAccessBits |= GL_MAP_ACCESS_BITS[accessBitShift];
            }
        }

        return glAccessBits;
    }

    GLbitfield calculateImmutableAccessBits(const uint16_t& AccessPolicy)
    {
        uint8_t accessBit = 1;
        GLbitfield glAccessBits = 0;

        for (uint8_t accessBitShift = 0;
             accessBitShift < sizeof(GL_IMMUTABLE_ACCESS_BITS) / sizeof(GLenum); ++accessBitShift)
        {
            if (AccessPolicy & (accessBit << accessBitShift))
            {
                glAccessBits |= GL_IMMUTABLE_ACCESS_BITS[accessBitShift];
            }
        }

        return glAccessBits;
    }

    GLBuffer::GLBuffer(const GLuint& BufferHandle, const BufferDescription& Description, const bool& IsImmutable)
    : glBufferHandle(BufferHandle), description(Description), isImmutable(IsImmutable)
    {
    }

    uint32_t GLBuffer::GetSize() const { return description.size; }

    void GLBuffer::Bind(const BufferBindPoint& BindPoint)
    {
        assert(glBufferHandle > 0);
        assert(currentBindPoint == BufferBindPoint::UNBOUND);

        currentBindPoint = BindPoint;

        glBindBuffer(AS_GL_BIND_POINT(BindPoint), glBufferHandle);
    }

    void GLBuffer::BindIndexed(const uint32_t& Index, const BufferBindPoint& BindPoint)
    {
        assert(glBufferHandle > 0);
        assert(currentBindPoint == BufferBindPoint::UNBOUND);

        currentBindPoint = BindPoint;

        glBindBufferBase(AS_GL_BIND_POINT(BindPoint), Index, glBufferHandle);
    }

    void GLBuffer::Download(void* Destination, uint32_t Size, const uint32_t& Offset)
    {
        assert(glBufferHandle > 0);

        if (Size < 1)
            Size = description.size;

        glBindBuffer(GL_COPY_READ_BUFFER, glBufferHandle);
        glGetBufferSubData(GL_COPY_READ_BUFFER, Offset, Size, Destination);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
    }

    void* GLBuffer::MemoryMap(const BufferBindPoint& BindPoint,
                              const uint16_t& AccessPolicy,
                              uint32_t Size,
                              const uint32_t& Offset)
    {
        assert(glBufferHandle > 0);
        assert(isImmutable ? true : NO_COHERENT_OR_PERSISTENT_BIT_SET(AccessPolicy));
        assert(currentBindPoint == BufferBindPoint::UNBOUND);

        currentBindPoint = BindPoint;

        if (Size < 1)
            Size = description.size;

        GLbitfield accessBits = calculateGLMapAccessBits(AccessPolicy);

        glBindBuffer(GL_COPY_READ_BUFFER, glBufferHandle);
        void* mappedRegion = glMapBufferRange(GL_COPY_READ_BUFFER, Offset, Size, accessBits);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        assert(mappedRegion);

        return mappedRegion;
    }

    void* GLBuffer::MemoryUnmap()
    {
        assert(currentBindPoint);
        glUnmapBuffer(AS_GL_BIND_POINT(currentBindPoint));
        currentBindPoint = BufferBindPoint::UNBOUND;
    }

    void GLBuffer::Upload(BufferDescription const* Description)
    {
        assert(glBufferHandle > 0);
        assert(!isImmutable);

        glBindBuffer(GL_COPY_READ_BUFFER, glBufferHandle);
        glBufferSubData(GL_COPY_READ_BUFFER, Description->offset, Description->size, Description->data);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);

        description = *Description;
    }

    void GLBuffer::Free()
    {
        assert(glBufferHandle > 0);
        glDeleteBuffers(1, &glBufferHandle);
    }

    Buffer* CreateBuffer(const BufferDescription& Description, const BufferUsage& Usage)
    {
        GLuint bufferHandle;

        glGenBuffers(1, &bufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, bufferHandle);
        glBufferData(GL_COPY_WRITE_BUFFER, Description.size, Description.data,
                     GL_MUTABLE_USAGE_HINTS[(uint16_t)Usage]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        return new GLBuffer(bufferHandle, Description, false);
    };

    Buffer* CreateImmutableBuffer(const BufferDescription& Description, const uint16_t& ImmutableBufferUsage)
    {
        GLuint bufferHandle;

        glGenBuffers(1, &bufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, bufferHandle);
        glBufferStorage(GL_COPY_WRITE_BUFFER, Description.size, Description.data,
                        calculateImmutableAccessBits(ImmutableBufferUsage));
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        return new GLBuffer(bufferHandle, Description, true);
    };
} // namespace lucid::gpu