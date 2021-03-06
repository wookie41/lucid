#pragma once

#include "devices/gpu/vao.hpp"
#include "GL/glew.h"

namespace lucid::gpu
{
    class GLVertexArray : public VertexArray
    {
      public:
        GLVertexArray(const GLuint& GLVAOHandle,
                      const DrawMode& DrawMode,
                      const uint32_t& VertexCount,
                      const uint32_t& ElementCount,
                      Buffer* VertexBuffer,
                      Buffer* ElementBuffer,
                      const bool& AutoDestroyBuffers);

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void EnableAttribute(const uint32_t& AttributeIndex) override;
        virtual void DisableAttribute(const uint32_t& AttributeIndex) override;

        virtual void AddVertexAttribute(const VertexAttribute& Attribute) override;
        virtual void AddIntegerVertexAttribute(const VertexAttribute& Attribute) override;
        virtual void AddLongVertexAttribute(const VertexAttribute& Attribute) override;

        virtual void SetVertexCount(const uint32_t& Count) override;
        virtual uint32_t GetVertexCount() const override;

        virtual void SetElementCount(const uint32_t& Count) override;
        virtual uint32_t GetElementCount() const override;

        virtual void Draw(const uint32_t& First, const uint32_t& Count) override;
        virtual void DrawInstanced(const uint32_t& InstancesCount,
                                   const uint32_t& First = 0,
                                   const uint32_t& Count = 0) override;

        virtual void Free() override;

        virtual ~GLVertexArray() = default;

      private:
        DrawMode drawMode;

        uint32_t vertexCount;
        uint32_t elementCount;

        bool autoDestroyBuffers;
        Buffer* vertexBuffer;
        Buffer* elementBuffer;

        GLuint glVAOHandle;
    };
} // namespace lucid::gpu
