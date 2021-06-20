#include "devices/gpu/gl/gl_pixelbuffer.hpp"

#include "devices/gpu/pixelbuffer.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/fence.hpp"

namespace lucid::gpu
{
    CPixelBuffer* CreatePixelBuffer(const FString& InName, const u32& InSizeInBytes)
    {
        GLuint GLHandle;
        glGenBuffers(1, &GLHandle);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, GLHandle);
        glBufferData(GL_PIXEL_PACK_BUFFER, InSizeInBytes, nullptr, GL_DYNAMIC_DRAW);
        return new CGLPixelBuffer(InName, GLHandle);
    }

    CGLPixelBuffer::CGLPixelBuffer(const FString& InName, const GLuint& InGLHandle) : CPixelBuffer(InName) { GLHandle = InGLHandle; }

    void CGLPixelBuffer::AsyncReadPixels(const u8&          InAttachmentIdx,
                                         const u16&         InX,
                                         const u16&         InY,
                                         const u16&         InWidth,
                                         const u16&         InHeight,
                                         gpu::CFramebuffer* InFramebuffer)
    {
        assert(GLHandle);
        assert(!ReadFence || ReadFence->IsSignaled());
        glBindBuffer(GL_PIXEL_PACK_BUFFER, GLHandle);

        InFramebuffer->Bind(EFramebufferBindMode::READ_WRITE);
        InFramebuffer->ReadPixels(InAttachmentIdx, InX, InY, InWidth, InHeight, nullptr);

        if (ReadFence)
        {
            ReadFence->Free();
        }

        ReadFence = CreateFence("PixelTransferFence");
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
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

        char* Pixels = nullptr;
        switch (InMapMode)
        {
        case EMapMode::READ_ONLY:
            glBindBuffer(GL_PIXEL_PACK_BUFFER, GLHandle);

            Pixels           = (char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            CurrentBindPoint = GL_PIXEL_PACK_BUFFER;

            break;
        case EMapMode::WRITE_ONLY:
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, GLHandle);

            Pixels           = (char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
            CurrentBindPoint = GL_PIXEL_UNPACK_BUFFER;

            break;
        case EMapMode::READ_WRITE:
            assert(0);
        }


        assert(Pixels);
        return Pixels;
    }

    void CGLPixelBuffer::UnmapBuffer()
    {
        glUnmapBuffer(CurrentBindPoint);
        glBindBuffer(CurrentBindPoint, 0);
    }

    void CGLPixelBuffer::SetObjectName() {}

    void CGLPixelBuffer::Free()
    {
        assert(GLHandle);
        glDeleteBuffers(1, &GLHandle);
    }
} // namespace lucid::gpu
