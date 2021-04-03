#pragma once

#include <glm/vec2.hpp>


#include "viewport.hpp"
#include "common/types.hpp"

namespace lucid::gpu
{
    /////////////////////////////////////
    //           Buffers               //
    /////////////////////////////////////
    enum EGPUBuffer : u8
    {
        COLOR = 1,
        DEPTH = 2,
        ACCUMULATION = 4,
        STENCIL = 8
    };

    void ClearBuffers(const EGPUBuffer& BuffersToClear);
    void SetClearColor(const FColor& Color);
    void SetClearDepth(const float& DepthValue);

    /////////////////////////////////////
    //           Depth tests          //
    /////////////////////////////////////

    enum class EDepthTestFunction : u8
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
    void SetDepthTestFunction(const EDepthTestFunction& Function);
    void SetReadOnlyDepthBuffer(const bool& InReadOnly);

    /////////////////////////////////////
    //           Blending              //
    /////////////////////////////////////

    enum class EBlendFunction : u8
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
    void SetBlendFunction(const EBlendFunction& SrcFunction, const EBlendFunction& DstFunction);
    void SetBlendFunctionSeparate(const EBlendFunction& SrcFunction,
                                  const EBlendFunction& DstFunction,
                                  const EBlendFunction& SrcAlphaFunction,
                                  const EBlendFunction& DstAlphaFunction);

    void EnableBlending();
    void DisableBlending();

    /////////////////////////////////////
    //            Culling              //
    /////////////////////////////////////

    enum class ECullMode : u8
    {
        FRONT,
        BACK,
        FRONT_AND_BACK
    };

    void EnableCullFace();
    void DisableCullFace();

    void SetCullMode(ECullMode Mode);

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

    struct FPipelineState
    {
        FColor  ClearColorBufferColor   = {0, 0, 0, 1};
        float   ClearDepthBufferValue   = 0;
                
        bool                IsDepthTestEnabled  = false;
        EDepthTestFunction  DepthTestFunction   = EDepthTestFunction::LEQUAL;
        
        bool                IsBlendingEnabled = false;
        EBlendFunction      BlendFunctionSrc;
        EBlendFunction      BlendFunctionDst;
        EBlendFunction      BlendFunctionAlphaSrc;
        EBlendFunction      BlendFunctionAlphaDst;
        
        bool                IsCullingEnabled = false;
        ECullMode           CullMode;
        
        bool                IsSRGBFramebufferEnabled = false;
        bool                IsDepthBufferReadOnly = false;

        FViewport           Viewport;
    };
    
    struct FGPUState
    {
        CFramebuffer*   Framebuffer      = nullptr;
        CFramebuffer*   ReadFramebuffer  = nullptr;
        CFramebuffer*   WriteFramebuffer = nullptr;

        CCubemap*       Cubemap         = nullptr;
        CShader*        Shader          = nullptr;
        CTexture**      BoundTextures   = nullptr;
        CRenderbuffer*  Renderbuffer    = nullptr;
        CVertexArray*   VAO             = nullptr;

        CBuffer*    VertexBuffer        = nullptr;
        CBuffer*    ElementBuffer       = nullptr;
        CBuffer*    ReadBuffer          = nullptr;
        CBuffer*    WriteBuffer         = nullptr;
        CBuffer*    ShaderStorageBuffer = nullptr;

       FPipelineState PipelineState; 
    };

    void ConfigurePipelineState(const FPipelineState& InPipelineState);
    
    struct FGPUInfo
    {
        u32 ActiveTextureUnit = 0;
        u32 MaxTextureUnits = 0;
        u32 MaxColorAttachments = 0;
    };

    extern FGPUInfo Info;
    inline thread_local FGPUState* GPUState;

    /** Forces all of the command to finish before the next on is executed */
    void Finish();
} // namespace lucid::gpu
