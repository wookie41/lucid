#pragma once

#include "devices/gpu/vao.hpp"
#include "glad/glad.h"

namespace lucid::gpu
{
    class CGLVertexArray : public CVertexArray
    {
      public:
        CGLVertexArray(const FString& InName,
                       const GLuint& InGLVAOHandle,
                       const EDrawMode& InDrawMode,
                       const uint32_t& InVertexCount,
                       const uint32_t& InElementCount,
                       CGPUBuffer* InVertexBuffer,
                       CGPUBuffer* InElementBuffer,
                       const bool& InAutoDestroyBuffers,
                       const FArray<FVertexAttribute>& InVertexAttributes);

        void SetObjectName() override;
        
        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void EnableAttribute(const uint32_t& AttributeIndex) override;
        virtual void DisableAttribute(const uint32_t& AttributeIndex) override;

        virtual void AddVertexAttribute(const FVertexAttribute& Attribute) override;
        virtual void AddIntegerVertexAttribute(const FVertexAttribute& Attribute) override;
        virtual void AddLongVertexAttribute(const FVertexAttribute& Attribute) override;

        virtual void SetVertexCount(const uint32_t& Count) override;
        virtual uint32_t GetVertexCount() const override;

        virtual void SetElementCount(const uint32_t& Count) override;
        virtual uint32_t GetElementCount() const override;

        virtual void Draw(const uint32_t& First, const uint32_t& Count) override;
        virtual void DrawInstanced(const uint32_t& InstancesCount,
                                   const uint32_t& First = 0,
                                   const uint32_t& Count = 0) override;

        virtual CGPUBuffer* GetVertexBuffer() const override;
        virtual void        SetVertexBuffer(CGPUBuffer* InVertexBuffer) override;

        virtual void SetupVertexAttributes() override;

        
        virtual void Free() override;

        virtual ~CGLVertexArray() = default;

      private:
        EDrawMode DrawMode;

        uint32_t VertexCount;
        uint32_t ElementCount;

        bool AutoDestroyBuffers;
        CGPUBuffer* VertexBuffer;
        CGPUBuffer* ElementBuffer;

        GLuint GLVAOHandle;

        FArray<FVertexAttribute> VertexAttributes;
    };
} // namespace lucid::gpu
