#include "devices/gpu/gl/gl_vao.hpp"

#include <scene/renderer.hpp>

#include "devices/gpu/shader.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/gl/gl_common.hpp"
#include "devices/gpu/gpu.hpp"

#ifndef NDEBUG
#include <cassert>
#endif

GLenum GL_DRAW_MODES[] = { GL_POINTS,
                           GL_LINE_STRIP,
                           GL_LINE_LOOP,
                           GL_LINES,
                           GL_LINE_STRIP_ADJACENCY,
                           GL_LINES_ADJACENCY,
                           GL_TRIANGLE_STRIP,
                           GL_TRIANGLE_FAN,
                           GL_TRIANGLES,
                           GL_TRIANGLE_STRIP_ADJACENCY,
                           GL_TRIANGLES_ADJACENCY,
                           GL_PATCHES };

namespace lucid::gpu
{
    CVertexArray* CreateVertexArray(const FString&            InName,
                                    FArray<FVertexAttribute>* VertexArrayAttributes,
                                    CGPUBuffer*               VertexBuffer,
                                    CGPUBuffer*               ElementBuffer,
                                    const EDrawMode&          DrawMode,
                                    const u32&                VertexCount,
                                    const u32&                ElementCount,
                                    const bool&               AutoDestroyBuffers)
    {
        GLuint VAO;

        glGenVertexArrays(1, &VAO);

        CVertexArray* vertexArray =
          new CGLVertexArray(InName, VAO, DrawMode, VertexCount, ElementCount, VertexBuffer, ElementBuffer, AutoDestroyBuffers);
        vertexArray->Bind();

        if (VertexBuffer)
        {
            VertexBuffer->Bind(EBufferBindPoint::VERTEX);
        }

        if (ElementBuffer != nullptr)
            ElementBuffer->Bind(EBufferBindPoint::ELEMENT);

        for (u32 attrIdx = 0; attrIdx < VertexArrayAttributes->GetLength(); ++attrIdx)
        {
            FVertexAttribute* vertexAttribute = (*VertexArrayAttributes)[attrIdx];

            switch (vertexAttribute->AttributeType)
            {
            case lucid::EType::FLOAT:
                vertexArray->AddVertexAttribute(*vertexAttribute);
                break;
            case lucid::EType::UINT_32:
            case lucid::EType::INT_32:
                vertexArray->AddIntegerVertexAttribute(*vertexAttribute);
                break;
            case lucid::EType::DOUBLE:
                vertexArray->AddLongVertexAttribute(*vertexAttribute);
                break;
            default:
                assert(0);
                break;
            }
        }

        vertexArray->Unbind();

        vertexArray->SetObjectName();
        return vertexArray;
    }

#ifdef LINUX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
    void CGLVertexArray::AddVertexAttribute(const FVertexAttribute& Attribute)
    {
        if (Attribute.BufferBindingIndex == -1)
        {
            glVertexAttribPointer(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], GL_FALSE, Attribute.Stride, (void*)Attribute.FirstElementOffset);
        }
        else
        {
            glVertexAttribFormat(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.Normalized, Attribute.FirstElementOffset);
            glVertexAttribBinding(Attribute.Index, Attribute.BufferBindingIndex);
        }
        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
        glEnableVertexAttribArray(Attribute.Index);
    }

    void CGLVertexArray::AddIntegerVertexAttribute(const FVertexAttribute& Attribute)
    {
        if (Attribute.BufferBindingIndex == -1)
        {
            glVertexAttribIPointer(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.Stride, (void*)Attribute.FirstElementOffset);
        }
        else
        {
            glVertexAttribIFormat(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.FirstElementOffset);
            glVertexAttribBinding(Attribute.Index, Attribute.BufferBindingIndex);
        }
        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
        glEnableVertexAttribArray(Attribute.Index);
    }

    void CGLVertexArray::AddLongVertexAttribute(const FVertexAttribute& Attribute)
    {
        if (Attribute.BufferBindingIndex == -1)
        {
            glVertexAttribLPointer(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.Stride,(void*)Attribute.FirstElementOffset);
        }
        else
        {
            glVertexAttribLFormat(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.FirstElementOffset);
            glVertexAttribBinding(Attribute.Index, Attribute.BufferBindingIndex);
        }
        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
        glEnableVertexAttribArray(Attribute.Index);
    }

#pragma GCC diagnostic pop

    CGLVertexArray::CGLVertexArray(const FString&   InName,
                                   const GLuint&    InGLVAOHandle,
                                   const EDrawMode& InDrawMode,
                                   const u32&       InVertexCount,
                                   const u32&       InElementCount,
                                   CGPUBuffer*      InVertexBuffer,
                                   CGPUBuffer*      InElementBuffer,
                                   const bool&      InAutoDestroyBuffers)
    : CVertexArray(InName), GLVAOHandle(InGLVAOHandle), DrawMode(InDrawMode), VertexBuffer(InVertexBuffer), ElementBuffer(InElementBuffer),
      AutoDestroyBuffers(InAutoDestroyBuffers), VertexCount(InVertexCount), ElementCount(InElementCount)
    {
    }

    void CGLVertexArray::SetObjectName() { SetGLObjectName(GL_VERTEX_ARRAY, GLVAOHandle, Name); }

    void CGLVertexArray::Free()
    {
        assert(GLVAOHandle && VertexBuffer); // double free

        if (AutoDestroyBuffers)
        {
            VertexBuffer->Free();
            if (ElementBuffer)
            {
                ElementBuffer->Free();
                ElementBuffer = nullptr;
            }
            VertexBuffer = nullptr;
        }

        glDeleteVertexArrays(1, &GLVAOHandle);
        GLVAOHandle = 0;
    }

    void CGLVertexArray::Bind()
    {
        GGPUState->VAO = this;
        glBindVertexArray(GLVAOHandle);
    }

    void CGLVertexArray::Unbind() { glBindVertexArray(0); }

    void CGLVertexArray::EnableAttribute(const u32& AttributeIndex) { glEnableVertexArrayAttrib(GLVAOHandle, AttributeIndex); }

    void CGLVertexArray::DisableAttribute(const u32& AttributeIndex) { glDisableVertexArrayAttrib(GLVAOHandle, AttributeIndex); }

    void CGLVertexArray::Draw(const u32& First, const u32& Count)
    {
        assert(GGPUState->VAO == this);
        u32 count = Count == 0 ? (ElementBuffer == nullptr ? VertexCount : ElementCount) : Count;

#if DEVELOPMENT
        ++scene::GRenderStats.NumDrawCalls;
#endif

        if (ElementBuffer)
        {
            glDrawElements(GL_DRAW_MODES[DrawMode], count, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_DRAW_MODES[DrawMode], First, count);
        }
    }

    void CGLVertexArray::DrawInstanced(const u32& InstancesCount, const uint32_t& First, const uint32_t& Count)
    {
        assert(GGPUState->VAO == this);
        uint32_t count = Count == 0 ? (ElementBuffer == nullptr ? VertexCount : ElementCount) : Count;
        ++scene::GRenderStats.NumDrawCalls;

        if (ElementBuffer)
        {
            glDrawElementsInstanced(GL_DRAW_MODES[DrawMode], count, GL_UNSIGNED_INT, nullptr, InstancesCount);
        }
        else
        {
            glDrawArraysInstanced(GL_DRAW_MODES[DrawMode], First, count, InstancesCount);
        }
    }

    void CGLVertexArray::SetVertexCount(const uint32_t& Count) { VertexCount = Count; }

    uint32_t CGLVertexArray::GetVertexCount() const { return VertexCount; }

    void CGLVertexArray::SetElementCount(const uint32_t& Count) { ElementCount = Count; }

    uint32_t CGLVertexArray::GetElementCount() const { return ElementCount; }

} // namespace lucid::gpu