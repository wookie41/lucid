#include "devices/gpu/gl/gl_buffer.hpp"
#include "devices/gpu/gpu.hpp"

#include <cassert>

#include "devices/gpu/gl/gl_common.hpp"

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

    GLbitfield CalculateImmutableAccessBits(const EImmutableBufferUsage& AccessPolicy)
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

    CGLBuffer::CGLBuffer(const GLuint& InGLBufferHandle, const FBufferDescription& InBufferDescription, const bool& InbImmutable, const FString& InName)
    : CBuffer(InName), GLBufferHandle(InGLBufferHandle), BufferDescription(InBufferDescription), bImmutable(InbImmutable)
    {
    }

    uint32_t CGLBuffer::GetSize() const { return BufferDescription.Size; }

    void CGLBuffer::SetObjectName()
    {
        SetGLObjectName(GL_BUFFER, GLBufferHandle, Name);
    }

    void CGLBuffer::Bind(const EBufferBindPoint& BindPoint)
    {
        assert(GLBufferHandle);
        CurrentBindPoint = BindPoint;
        glBindBuffer(AS_GL_BIND_POINT(BindPoint), GLBufferHandle);
    }

    void CGLBuffer::BindIndexed(const uint32_t& Index, const EBufferBindPoint& BindPoint)
    {
        assert(GLBufferHandle);
        CurrentBindPoint = BindPoint;
        glBindBufferBase(AS_GL_BIND_POINT(BindPoint), Index, GLBufferHandle);
    }

    void CGLBuffer::Download(void* Destination, uint32_t Size, const uint32_t& Offset)
    {
        assert(GLBufferHandle);

        if (Size < 1)
            Size = BufferDescription.Size;

        glBindBuffer(GL_COPY_READ_BUFFER, GLBufferHandle);
        glGetBufferSubData(GL_COPY_READ_BUFFER, Offset, Size, Destination);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
    }

    void* CGLBuffer::MemoryMap(const EBufferBindPoint& BindPoint,
                               const EBufferAccessPolicy& AccessPolicy,
                               uint32_t Size,
                               const uint32_t& Offset)
    {
        assert(GLBufferHandle);
        assert(bImmutable ? true : NO_COHERENT_OR_PERSISTENT_BIT_SET(AccessPolicy));

        gpu::CBuffer** Target = nullptr;
        switch (BindPoint)
        {
        case EBufferBindPoint::VERTEX:
            Target = &GGPUState->VertexBuffer;
            break;
        case EBufferBindPoint::ELEMENT:
            Target = &GGPUState->ElementBuffer;
            break;
        case EBufferBindPoint::SHADER_STORAGE:
            Target = &GGPUState->ShaderStorageBuffer;
            break;
        default:
            break;
        }

        assert(*Target != nullptr);
        *Target = this;

        CurrentBindPoint = BindPoint;

        if (Size < 1)
            Size = BufferDescription.Size;

        GLbitfield AccessBits = calculateGLMapAccessBits(AccessPolicy);

        CurrentBindPoint = BindPoint;
        GLenum glBindPoint = AS_GL_BIND_POINT(AccessBits);

        glBindBuffer(glBindPoint, GLBufferHandle);
        void* MappedRegion = glMapBufferRange(glBindPoint, Offset, Size, AccessBits);

        if (bImmutable)
            *Target = nullptr;
        glBindBuffer(glBindPoint, 0);

        assert(MappedRegion);

        return MappedRegion;
    }

    void CGLBuffer::MemoryUnmap()
    {
        gpu::CBuffer** Target = nullptr;
        switch (CurrentBindPoint)
        {
        case EBufferBindPoint::VERTEX:
            Target = &GGPUState->VertexBuffer;
            break;
        case EBufferBindPoint::ELEMENT:
            Target = &GGPUState->ElementBuffer;
            break;
        case EBufferBindPoint::SHADER_STORAGE:
            Target = &GGPUState->ShaderStorageBuffer;
            break;
        default:
            break;
        }

        // check if this buffer is actually bound to the place it says it's

        assert(*Target == this);
        *Target = nullptr;

        glUnmapBuffer(AS_GL_BIND_POINT(CurrentBindPoint));
        CurrentBindPoint = EBufferBindPoint::UNBOUND;
    }

    void CGLBuffer::Upload(FBufferDescription const* Description)
    {
        assert(GLBufferHandle);
        assert(!bImmutable);

        glBindBuffer(GL_COPY_READ_BUFFER, GLBufferHandle);
        glBufferSubData(GL_COPY_READ_BUFFER, Description->Offset, Description->Size, Description->Data);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);

        BufferDescription = *Description;
    }

    void CGLBuffer::Free()
    {
        assert(GLBufferHandle);
        glDeleteBuffers(1, &GLBufferHandle);
        GLBufferHandle = 0;
    }

    CBuffer* CreateBuffer(const FBufferDescription& Description, const EBufferUsage& Usage, const FString& InName)
    {
        GLuint bufferHandle;

        glGenBuffers(1, &bufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, bufferHandle);
        glBufferData(GL_COPY_WRITE_BUFFER, Description.Size, Description.Data, GL_MUTABLE_USAGE_HINTS[(u16)Usage]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        auto* GLBuffer = new CGLBuffer(bufferHandle, Description, false, InName);
        GLBuffer->SetObjectName();
        return GLBuffer;
    };

    CBuffer* CreateImmutableBuffer(const FBufferDescription& Description, const EImmutableBufferUsage& ImmutableBufferUsage, const FString& InName)
    {
        GLuint BufferHandle;

        glGenBuffers(1, &BufferHandle);
        glBindBuffer(GL_COPY_WRITE_BUFFER, BufferHandle);
        glBufferStorage(GL_COPY_WRITE_BUFFER, Description.Size, Description.Data, CalculateImmutableAccessBits(ImmutableBufferUsage));
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        auto* GLBuffer = new CGLBuffer(BufferHandle, Description, true, InName);
        GLBuffer->SetObjectName();
        return GLBuffer;
    };
} // namespace lucid::gpu