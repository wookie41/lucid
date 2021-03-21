#include "devices/gpu/gpu.hpp"


#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "GL/glew.h"
#include "misc/basic_shapes.hpp"

namespace lucid::gpu
{

    // Buffers functions
    void ClearBuffers(const ClearableBuffers& BuffersToClear)
    {
        static GLbitfield buffersBits[] = { GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_ACCUM_BUFFER_BIT,
                                            GL_STENCIL_BUFFER_BIT };

        GLbitfield glBuffersBitField = 0;
        if (BuffersToClear & ClearableBuffers::COLOR)
        {
            glBuffersBitField |= GL_COLOR_BUFFER_BIT;
        }

        if (BuffersToClear & ClearableBuffers::DEPTH)
        {
            glBuffersBitField |= GL_DEPTH_BUFFER_BIT;
        }

        if (BuffersToClear & ClearableBuffers::STENCIL)
        {
            glBuffersBitField |= GL_COLOR_BUFFER_BIT;
        }

        if (BuffersToClear & ClearableBuffers::ACCUMULATION)
        {
            glBuffersBitField |= GL_ACCUM_BUFFER_BIT;
        }

        glClear(glBuffersBitField);
    }

    void SetClearColor(const FColor& Color) { glClearColor(Color.r, Color.g, Color.b, Color.a); }

    void SetClearDepth(const float& DepthValue) { glClearDepth(DepthValue); }

    /////////////////////////////////////////////////////////

    // Depth test functions

    static const GLenum GL_DEPTH_FUNCTIONS[] = { GL_NEVER,   GL_LESS,     GL_EQUAL,  GL_LEQUAL,
                                                 GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };

    void EnableDepthTest() { glEnable(GL_DEPTH_TEST); }

    void DisableDepthTest() { glDisable(GL_DEPTH_TEST); }

    void SetReadOnlyDepthBuffer(const bool& InReadOnly)
    {
        glDepthMask(!InReadOnly);
    }
    
    void SetDepthTestFunction(const DepthTestFunction& Function)
    {
        glDepthFunc(GL_DEPTH_FUNCTIONS[static_cast<u8>(Function)]);
    }

    ////////////////////////////////////////////////////////////

    // Blending

    static const GLenum GL_BLEND_ENUMS[] = { GL_ZERO,
                                             GL_ONE,
                                             GL_SRC_COLOR,
                                             GL_ONE_MINUS_SRC_COLOR,
                                             GL_DST_COLOR,
                                             GL_ONE_MINUS_DST_COLOR,
                                             GL_SRC_ALPHA,
                                             GL_ONE_MINUS_SRC_ALPHA,
                                             GL_DST_ALPHA,
                                             GL_ONE_MINUS_DST_ALPHA,
                                             GL_CONSTANT_COLOR,
                                             GL_ONE_MINUS_CONSTANT_COLOR,
                                             GL_CONSTANT_ALPHA,
                                             GL_ONE_MINUS_CONSTANT_ALPHA,
                                             GL_SRC_ALPHA_SATURATE,
                                             GL_SRC1_COLOR,
                                             GL_ONE_MINUS_SRC1_COLOR,
                                             GL_SRC1_ALPHA,
                                             GL_ONE_MINUS_SRC1_ALPHA };

#define TO_GL_BLEND(blendMode) GL_BLEND_ENUMS[static_cast<u8>(blendMode)]

    void SetBlendColor(const FColor& Color) { glBlendColor(Color.r, Color.g, Color.b, Color.a); }

    void SetBlendFunction(const BlendFunction& SrcFunction, const BlendFunction& DstFunction)
    {
        glBlendFunc(TO_GL_BLEND(SrcFunction), TO_GL_BLEND(DstFunction));
    }

    void SetBlendFunctionSeparate(const BlendFunction& SrcFunction,
                                  const BlendFunction& DstFunction,
                                  const BlendFunction& SrcAlphaFunction,
                                  const BlendFunction& DstAlphaFunction)

    {
        glBlendFuncSeparate(TO_GL_BLEND(SrcFunction), TO_GL_BLEND(DstFunction), TO_GL_BLEND(SrcAlphaFunction),
                            TO_GL_BLEND(DstAlphaFunction));
    }

    void EnableBlending() { glEnable(GL_BLEND); }

    void DisableBlending() { glDisable(GL_BLEND); }

    //////////////////////////////////////////////////////

    ///////////// Culling //////////////////////

    static const GLenum CULL_MODE_MAPPING[]{ GL_FRONT, GL_BACK, GL_FRONT_AND_BACK };
    
    void EnableCullFace()
    {
        glEnable(GL_CULL_FACE);
    }

    void DisableCullFace()
    {
        glDisable(GL_CULL_FACE);
    }

    void SetCullMode(CullMode Mode)
    {
        glCullFace(CULL_MODE_MAPPING[static_cast<u8>(Mode)]);
    }

    // SRGB

    void EnableSRGBFramebuffer() { glEnable(GL_FRAMEBUFFER_SRGB); }

    void DisableSRGBFramebuffer() { glDisable(GL_FRAMEBUFFER_SRGB); }

    //////////////////////////////////////////////////////

    static const FString QUAD_POSITION ("uQuadPosition");
    static const FString QUAD_SIZE ("uQuadSize");
    
    void DrawImmediateQuad(const glm::vec2& InPosition, const glm::vec2& InSize)
    {
        assert(Info.CurrentShader);
        misc::QuadVertexArray->Bind();
        Info.CurrentShader->SetVector(QUAD_POSITION, InPosition);
        Info.CurrentShader->SetVector(QUAD_SIZE, InSize);
        misc::QuadVertexArray->Draw();
    }
    
} // namespace lucid::gpu