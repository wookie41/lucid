#include "devices/gpu/gl/buffer.hpp"
#include "devices/gpu/gpu.hpp"

#include <cassert>

#define NO_COHERENT_OR_PERSISTENT_BIT_SET(FLAGS) \
    (((FLAGS & (u16)EBufferAccessPolicy::BUFFER_COHERENT) | (FLAGS & (u16)EBufferAccessPolicy::BUFFER_PERSISTENT)) == 0)

#define AS_GL_BIND_POINT(bindPoint) (GL_BIND_POINTS[((u16)bindPoint) - 1])

static const GLenum GL_BIND_POINTS[] = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER };

static const GLenum GL_MUTABLE_USAGE_HINTS[] = { GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER };

static const GLenum GL_IMMUTABLE_ACCESS_BITS[] = { GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT,     GL_MAP_WRITE_BIT,
                                                   GL_MAP_PERSISTENT_BIT,  GL_MAP_COHERENT_BIT, GL_CLIENT_STORAGE_BIT };

static const GLenum GL_MAP_ACCESS_BITS[] = { GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT };

namespace lucid::gpu
{
    GLbitfield calculateGLMapAccessBits(const EBufferAccessPolicy& MapPolicy)
    {
        u8 accessBit = 1;
        GLbitfield glAccessBits = 0;

        for (u8 accessBitShift = 0; accessBitShift < sizeof(GL_MAP_ACCESS_BITS) / sizeof(GLenum); ++accessBitShift)
        {
            if (MapPolicy & (accessBit << accessBitShift))
            {
                glAccessBits |= GL_MAP_ACCESS_BITS[accessBitShift];
            }
        }

        return glAccessBits;
    }

    GLbitfield calculateImmutableAccessBits(const EImmutableBufferUsage& AccessPolicy)
    {
        u8 accessBit = 1;
        GLbitfield glAccessBits = 0;

        for (u8 accessBitShift = 0; accessBitShift < sizeof(GL_IMMUTABLE_ACCESS_BITS) / sizeof(GLenum); ++accessBitShift)
        {
            if (AccessPolicy & (accessBit << accessBitShift))
            {
                glAccessBits |= GL_IMMUTABLE_ACCESS_BITS[accessBitShift];
            }
        }

        return glAccessBits;
    }

    CGLBuffer::CGLBuffer(const GLuint& BufferHandle,
                         const FBufferDescription& Description,
                         const bool& IsImmutable,
                         const FANSIString& InName,
                         FGPUState* InGPUState)
    : CBuffer(InName, InGPUState), glBufferHandle(BufferHandle), description(Description), isImmutable(IsImmutable)
    {
    }

    uint32_t CGLBuffer::GetSize() const { return description.size; }

    void CGLBuffer::Bind(const EBufferBindPoint& BindPoint)
    {
        assert(glBufferHandle > 0);

        currentBindPoint = BindPoint;

        glBindBuffer(AS_GL_BIND_POINT(BindPoint), glBufferHandle);
    }

    void CGLBuffer::BindIndexed(const uint32_t& Index, const EBufferBindPoint& BindPoint)
    {
        assert(glBufferHandle > 0);

        currentBindPoint = BindPoint;

        glBindBufferBase(AS_GL_BIND_POINT(BindPoint), Index, glBufferHandle);
    }

    void CGLBuffer::Download(void* Destination, uint32_t Size, const uint32_t& Offset)
    {
        assert(glBufferHandle > 0);

        if (Size < 1)
            Size = description.size;

        glBindBuffer(GL_COPY_READ_BUFFER, glBufferHandle);
        glGetBufferSubData(GL_COPY_READ_BUFFER, Offset, Size, Destination);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
    }

    void* CGLBuffer::MemoryMap(const EBufferBindPoint& BindPoint,
                               const EBufferAccessPolicy& AccessPolicy,
                               uint32_t Size,
                               const uint32_t& Offset)
    {
        assert(glBufferHandle > 0);
        assert(isImmutable ? true : NO_COHERENT_OR_PERSISTENT_BIT_SET(AccessPolicy));

        gpu::CBuffer** target = nullptr;
        switch (BindPoint)
        {
        case EBufferBindPoint::VERTEX:
            target = &GPUState->VertexBuffer;
            break;
        case EBufferBindPoint::ELEMENT:
            target = &GPUState->ElementBuffer;
            break;
        case EBufferBindPoint::SHADER_STORAGE:
            target = &GPUState->ShaderStorageBuffer;
            break;
        default:
            break;
        }

        assert(*target != nullptr);
        *target = this;

        currentBindPoint = BindPoint;

        if (Size < 1)
            Size = description.size;

        GLbitfield accessBits = calculateGLMapAccessBits(AccessPolicy);

        currentBindPoint = BindPoint;
        GLenum glBindPoint = AS_GL_BIND_POINT(currentBindPoint);

        glBindBuffer(glBindPoint, glBufferHandle);
        void* mappedRegion = glMapBufferRange(glBindPoint, Offset, Size, accessBits);

        if (isImmutable)
            *target = nullptr;
        glBindBuffer(glBindPoint, 0);

        assert(mappedRegion);

        return mappedRegion;
    }

    void CGLBuffer::MemoryUnmap()
    {
        gpu::CBuffer** target = nullptr;
        switch (currentBindPoint)
        {
        case EBufferBindPoint::VERTEX:
            target = &GPUState->VertexBuffer;
            break;
        case EBufferBindPoint::ELEMENT:
            target = &GPUState->ElementBuffer;
            break;
        case EBufferBindPoint::SHADER_STORAGE:
            target = &GPUState->ShaderStorageBuffer;
            break;
        default:
            break;
        }

        // check if this buffer is actually bound to the place it says it's

        assert(*target == this);
        *target = nullptr;

        glUnmapBuffer(AS_GL_BIND_POINT(currentBindPoint));
        currentBindPoint = EBufferBindPoint::UNBOUND;
    }

    void CGLBuffer::Upload(FBufferDescription const* Description)
    {
        assert(glBufferHandle > 0);
        assert(!isImmutable);

        glBindBuffer(GL_COPY_READ_BUFFER, glBufferHandle);
        glBufferSubData(GL_COPY_READ_BUFFER, Description->offset, Description->size, Description->data);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);

        description = *Description;
    }

    void CGLBuffer::Free()
    {
        assert(glBufferHandle > 0);
        glDeleteBuffers(1, &glBufferHandle);
    }

    CBuffer* CreateBuffer(const FBufferDescription& Description, const EBufferUsage& Usage, const FANSIString& InName, FGPUState* InGPUState)
    {
        GLuint bufferHandle;

        glGenBuffers(1, &bufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, bufferHandle);
        glBufferData(GL_COPY_WRITE_BUFFER, Description.size, Description.data, GL_MUTABLE_USAGE_HINTS[(u16)Usage]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        return new CGLBuffer(bufferHandle, Description, false, InName, InGPUState);
    };

    CBuffer* CreateImmutableBuffer(const FBufferDescription& Description, const EImmutableBufferUsage& ImmutableBufferUsage, const FANSIString& InName, FGPUState* InGPUState)
    {
        GLuint bufferHandle;

        glGenBuffers(1, &bufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, bufferHandle);
        glBufferStorage(GL_COPY_WRITE_BUFFER, Description.size, Description.data,
                        calculateImmutableAccessBits(ImmutableBufferUsage));
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        return new CGLBuffer(bufferHandle, Description, true, InName, InGPUState);
    };
} // namespace lucid::gpu