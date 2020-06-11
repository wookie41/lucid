#pragma once

#include "devices/gpu/gpu.hpp"
#include "gl/glew.h"

namespace lucid::gpu
{
    class GLVertexArray : public VertexArray
    {
      public:
        GLVertexArray(const GLuint& GLVAOHandle);

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void EnableAttribute(const uint32_t& AttributeIndex) override;
        virtual void DisableAttribute(const uint32_t& AttributeIndex) override;

        virtual ~GLVertexArray();

      private:
        GLuint glVAOHandle;
    };

    VertexArray* CreateVertexArray(const StaticArray<VertexAttribute> const* VertexArrayAttributes,
                                   const Buffer const* VertexBuffer,
                                   const Buffer const* ElementBuffer);
} // namespace lucid::gpu