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

    enum class DepthTestFunction : uint8_t
    {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS
    };

    void EnableDepthTest();
    void DisableDepthTest();
    void SetDepthTestFunction(const DepthTestFunction& Function);

    /////////////////////////////////////////

    /////////////// Blending ///////////////

    enum class BlendFunction : uint8_t
    {
        ZERO,
        ONE,
        SOURCE_COLOR,
        ONE_MINUS_SRC_COLOR,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_SATURATE,
        SRC1_COLOR,
        ONE_MINUS_SRC1_COLOR,
        SRC1_ALPHA,
        ONE_MINUS_SRC1_ALPHA
    };

    void SetBlendColor(const color& Color);
    void SetBlendFunction(const BlendFunction& SrcFunction, const BlendFunction& DstFunction);
    void SetBlendFunctionSeparate(const BlendFunction& SrcFunction,
                                  const BlendFunction& DstFunction,
                                  const BlendFunction& SrcAlphaFunction,
                                  const BlendFunction& DstAlphaFunction);

    void EnableBlending();
    void DisableBlending();

    ///////////////////////////////////////

    ///////////// sRGB //////////////////////

    void EnableSRGBFramebuffer();
    void DisableSRGBFramebuffer();

    /////////////// GPU Info ///////////////

    // Queries the GPU for it's properties, like the maximum number of samplers
    // supported extension, which shader/framebuffer/texture and etc is currently bound

    class Framebuffer;
    class Shader;
    class Texture;
    class Renderbuffer;
    class Buffer;
    class VertexArray;
    class Cubemap;

    struct GPUInfo
    {
        Framebuffer* CurrentFramebuffer = nullptr;
        Framebuffer* CurrentReadFramebuffer = nullptr;
        Framebuffer* CurrentWriteFramebuffer = nullptr;

        Cubemap* CurrentCubemap = nullptr;
        Shader* CurrentShader = nullptr;
        Texture** BoundTextures = nullptr;
        Renderbuffer* CurrentRenderbuffer;
        VertexArray* CurrentVAO = nullptr;

        Buffer* CurrentVertexBuffer;
        Buffer* CurrentElementBuffer;
        Buffer* CurrentReadBuffer;
        Buffer* CurrentWriteBuffer;
        Buffer* CurrentShaderStorageBuffer;

        uint32_t ActiveTextureUnit = 0;
        uint32_t MaxTextureUnits = 0;
        uint32_t MaxColorAttachments = 0;
    };

    extern GPUInfo Info;
} // namespace lucid::gpu
