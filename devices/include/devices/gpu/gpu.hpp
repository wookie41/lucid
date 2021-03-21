#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

#include "common/types.hpp"

namespace lucid::gpu
{
    /////////////////////////////////////
    //           Buffers               //
    /////////////////////////////////////
    enum ClearableBuffers : u8
    {
        COLOR = 1,
        DEPTH = 2,
        ACCUMULATION = 4,
        STENCIL = 8
    };

    void ClearBuffers(const ClearableBuffers& BuffersToClear);
    void SetClearColor(const FColor& Color);
    void SetClearDepth(const float& DepthValue);

    /////////////////////////////////////
    //           Depth tests          //
    /////////////////////////////////////

    enum class DepthTestFunction : u8
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
    void SetReadOnlyDepthBuffer(const bool& InReadOnly);

    /////////////////////////////////////
    //           Blending              //
    /////////////////////////////////////

    enum class BlendFunction : u8
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

    void SetBlendColor(const FColor& Color);
    void SetBlendFunction(const BlendFunction& SrcFunction, const BlendFunction& DstFunction);
    void SetBlendFunctionSeparate(const BlendFunction& SrcFunction,
                                  const BlendFunction& DstFunction,
                                  const BlendFunction& SrcAlphaFunction,
                                  const BlendFunction& DstAlphaFunction);

    void EnableBlending();
    void DisableBlending();

    /////////////////////////////////////
    //            Culling              //
    /////////////////////////////////////

    enum class CullMode : u8
    {
        FRONT,
        BACK,
        FRONT_AND_BACK
    };

    void EnableCullFace();
    void DisableCullFace();

    void SetCullMode(CullMode Mode);

    /////////////////////////////////////
    //              sRGB               //
    /////////////////////////////////////

    void EnableSRGBFramebuffer();
    void DisableSRGBFramebuffer();

    /////////////////////////////////////
    //              GPU Info           //
    /////////////////////////////////////

    /*
     * Queries the GPU for it's properties, like the maximum number of samplers
     * supported extension, which shader/framebuffer/texture and etc is currently bound
     */

    class CFramebuffer;
    class CShader;
    class CTexture;
    class CRenderbuffer;
    class CBuffer;
    class CVertexArray;
    class CCubemap;

    struct GPUInfo
    {
        CFramebuffer* CurrentFramebuffer = nullptr;
        CFramebuffer* CurrentReadFramebuffer = nullptr;
        CFramebuffer* CurrentWriteFramebuffer = nullptr;

        CCubemap* CurrentCubemap = nullptr;
        CShader* CurrentShader = nullptr;
        CTexture** BoundTextures = nullptr;
        CRenderbuffer* CurrentRenderbuffer;
        CVertexArray* CurrentVAO = nullptr;

        CBuffer* CurrentVertexBuffer;
        CBuffer* CurrentElementBuffer;
        CBuffer* CurrentReadBuffer;
        CBuffer* CurrentWriteBuffer;
        CBuffer* CurrentShaderStorageBuffer;

        u32 ActiveTextureUnit = 0;
        u32 MaxTextureUnits = 0;
        u32 MaxColorAttachments = 0;
    };

    /////////////////////////////////////
    //     Immediate drawing           //
    /////////////////////////////////////

    void DrawImmediateQuad(const glm::vec2& InPosition, const glm::vec2& InSize);

    extern GPUInfo Info;
} // namespace lucid::gpu
