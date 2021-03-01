#include "devices/gpu/gl/vao.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/gl/common.hpp"
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
    VertexArray* CreateVertexArray(StaticArray<VertexAttribute>* VertexArrayAttributes,
                                   Buffer* VertexBuffer,
                                   Buffer* ElementBuffer,
                                   const DrawMode& DrawMode,
                                   const u32& VertexCount,
                                   const u32& ElementCount,
                                   const bool& AutoDestroyBuffers)
    {
        GLuint VAO;

        glGenVertexArrays(1, &VAO);

        VertexArray* vertexArray =
          new GLVertexArray(VAO, DrawMode, VertexCount, ElementCount, VertexBuffer, ElementBuffer, AutoDestroyBuffers);
        vertexArray->Bind();

        VertexBuffer->Bind(BufferBindPoint::VERTEX);

        if (ElementBuffer != nullptr)
            ElementBuffer->Bind(BufferBindPoint::ELEMENT);

        for (u32 attrIdx = 0; attrIdx < VertexArrayAttributes->Length; ++attrIdx)
        {
            VertexAttribute* vertexAttribute = (*VertexArrayAttributes)[attrIdx];

            switch (vertexAttribute->AttributeType)
            {
            case lucid::Type::FLOAT:
                vertexArray->AddVertexAttribute(*vertexAttribute);
                break;
            case lucid::Type::BYTE:
            case lucid::Type::UINT_8:
            case lucid::Type::UINT_16:
            case lucid::Type::UINT_32:
                vertexArray->AddIntegerVertexAttribute(*vertexAttribute);
                break;
            case lucid::Type::DOUBLE:
                vertexArray->AddLongVertexAttribute(*vertexAttribute);
                break;
            default:
                assert(0);
                break;
            }
        }

        vertexArray->Unbind();
        glBindVertexArray(0);

        return vertexArray;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    void GLVertexArray::AddVertexAttribute(const VertexAttribute& Attribute)
    {
        glEnableVertexAttribArray(Attribute.Index);
        glVertexAttribPointer(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], GL_FALSE,
                              Attribute.Stride, (void*)Attribute.FirstElementOffset);

        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
    }

    void GLVertexArray::AddIntegerVertexAttribute(const VertexAttribute& Attribute)
    {
        glEnableVertexAttribArray(Attribute.Index);
        glVertexAttribIPointer(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.Stride,
                               (void*)Attribute.FirstElementOffset);

        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
    }

    void GLVertexArray::AddLongVertexAttribute(const VertexAttribute& Attribute)
    {
        glEnableVertexAttribArray(Attribute.Index);
        glVertexAttribLPointer(Attribute.Index, Attribute.NumComponents, GL_TYPES[static_cast<u8>(Attribute.AttributeType)], Attribute.Stride,
                               (void*)Attribute.FirstElementOffset);

        glVertexAttribDivisor(Attribute.Index, Attribute.Divisor);
    }

#pragma GCC diagnostic pop

    GLVertexArray::GLVertexArray(const GLuint& GLVAOHandle,
                                 const DrawMode& DrawMode,
                                 const u32& VertexCount,
                                 const u32& ElementCount,
                                 Buffer* VertexBuffer,
                                 Buffer* ElementBuffer,
                                 const bool& AutoDestroyBuffers)
    : glVAOHandle(GLVAOHandle), drawMode(DrawMode), vertexBuffer(VertexBuffer), elementBuffer(ElementBuffer),
      autoDestroyBuffers(AutoDestroyBuffers), vertexCount(VertexCount), elementCount(ElementCount)
    {
    }

    void GLVertexArray::Free()
    {
        assert(glVAOHandle && vertexBuffer); // double free
        glDeleteVertexArrays(1, &glVAOHandle);

        if (autoDestroyBuffers)
        {
            vertexBuffer->Free();
            if (elementBuffer)
            {
                elementBuffer->Free();
                elementBuffer = nullptr;
            }
        }

        glVAOHandle = 0;
        vertexBuffer = nullptr;
    }

    void GLVertexArray::Bind()
    {
        gpu::Info.CurrentVAO = this;
        glBindVertexArray(glVAOHandle);
    }

    void GLVertexArray::Unbind() { glBindVertexArray(0); }

    void GLVertexArray::EnableAttribute(const u32& AttributeIndex)
    {
        glEnableVertexArrayAttrib(glVAOHandle, AttributeIndex);
    }

    void GLVertexArray::DisableAttribute(const u32& AttributeIndex)
    {
        glDisableVertexArrayAttrib(glVAOHandle, AttributeIndex);
    }

    void GLVertexArray::Draw(const u32& First, const u32& Count)
    {
        u32 count = Count == 0 ? (elementBuffer == nullptr ? vertexCount : elementCount) : Count;

        if (elementBuffer)
        {
            glDrawElements(GL_DRAW_MODES[drawMode], count, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_DRAW_MODES[drawMode], First, count);
        }
    }

    void GLVertexArray::DrawInstanced(const u32& InstancesCount, const uint32_t& First, const uint32_t& Count)
    {
        uint32_t count = Count == 0 ? (elementBuffer == nullptr ? vertexCount : elementCount) : Count;

        if (elementBuffer)
        {
            glDrawElementsInstanced(GL_DRAW_MODES[drawMode], count, GL_UNSIGNED_INT, nullptr, InstancesCount);
            ;
        }
        else
        {
            glDrawArraysInstanced(GL_DRAW_MODES[drawMode], First, count, InstancesCount);
        }
    }

    void GLVertexArray::SetVertexCount(const uint32_t& Count) { vertexCount = Count; }

    uint32_t GLVertexArray::GetVertexCount() const { return vertexCount; }

    void GLVertexArray::SetElementCount(const uint32_t& Count) { elementCount = Count; }

    uint32_t GLVertexArray::GetElementCount() const { return elementCount; }

} // namespace lucid::gpu