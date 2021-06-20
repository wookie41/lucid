#include "devices/gpu/gl/gl_pixelbuffer.hpp"

#include "devices/gpu/pixelbuffer.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/fence.hpp"

namespace lucid::gpu
{
    CGLPixelBuffer::CGLPixelBuffer(const FString& InName, const GLuint& InGLHandle) : CPixelBuffer(InName) { GLHandle = InGLHandle; }

    void CGLPixelBuffer::AsyncReadPixels(gpu::CTexture const* InTexture)
    {
        assert(GLHandle);
        assert(!ReadFence);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, GLHandle);

        InTexture->CopyPixels(nullptr, 0);
        ReadFence = CreateFence("PixelTransferFence");
    }

    bool CGLPixelBuffer::IsReady()
    {
        assert(ReadFence);

        if (ReadFence->IsSignaled())
        {
            return true;
        }

        return ReadFence->Wait(0);
    }

    char* CGLPixelBuffer::MapBuffer(const EMapMode& InMapMode)
    {
        assert(GLHandle);
        assert(ReadFence->IsSignaled());

        glBindBuffer(GL_PIXEL_PACK_BUFFER, GLHandle);
        switch (InMapMode)
        {
        case EMapMode::READ_ONLY:
            return (char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        case EMapMode::WRITE_ONLY:
            return (char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_WRITE_ONLY);
        case EMapMode::READ_WRITE:
            return (char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
        }
    }

    void CGLPixelBuffer::UnmapBuffer()
    {
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }
} // namespace lucid::gpu
