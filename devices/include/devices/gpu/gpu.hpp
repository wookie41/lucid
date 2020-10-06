#pragma once

#include <cstdint>
#include "common/types.hpp"

namespace lucid::gpu
{
    /////////////// Buffers ///////////////

    enum ClearableBuffers : uint8_t
    {
        COLOR = 1,
        DEPTH = 2,
        ACCUMULATION = 4,
        STENCIL = 8
    };

    void ClearBuffers(const ClearableBuffers& BuffersToClear);
    void SetClearColor(const color& Color);

    /////////////// Depth tests ///////////////

    void EnableDepthTest();
    void DisableDepthTest();

    /////////////// GPU Info ///////////////

    // Queries the GPU for it's properties, like the maximum number of samplers
    // supported extension, which shader/framebuffer/texture and etc is currently bound

    struct GPUInfo
    {
        class Framebuffer* CurrentFramebuffer = nullptr;
        class Framebuffer* CurrentReadFramebuffer = nullptr;
        class Framebuffer* CurrentWriteFramebuffer = nullptr;

        class Shader* CurrentShader = nullptr;
        class Texture** BoundTextures = nullptr;
        class Renderbuffer* CurrentRenderbuffer;

        class Buffer* CurrentVertexBuffer;
        class Buffer* CurrentElementBuffer;
        class Buffer* CurrentReadBuffer;
        class Buffer* CurrentWriteBuffer;
        class Buffer* CurrentShaderStorageBuffer;


        uint32_t ActiveTextureUnit = 0;
        uint32_t MaxTextureUnits = 0;
        uint32_t MaxColorAttachments = 0;
    };

    extern GPUInfo Info;
} // namespace lucid::gpu
