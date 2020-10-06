#pragma once

#include "common/strings.hpp"
#include "devices/gpu/buffer.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class Texture;

    struct BufferBinding
    {
        Buffer* BufferToUse = nullptr;
        BufferBindPoint BindPoint = BufferBindPoint::UNBOUND;
        int32_t Index = -1;

        bool IsValid = false;
    };

    class Shader
    {
      public:
        virtual void Use() = 0;
        virtual void Disable() = 0;

        virtual void SetupBuffersBindings() = 0;

        // Return -1 if uniform doesn't exist
        virtual int32_t GetIdForUniform(const String& UniformName) const = 0;

        virtual void SetInt(const int32_t& UniformId, const uint32_t& Value) = 0;
        virtual void SetFloat(const int32_t& UniformId, const float& Value) = 0;
        virtual void SetBool(const int32_t& UniformId, const bool& Value) = 0;

        virtual void SetVector(const int32_t& UniformId, const glm::vec2& Value) = 0;
        virtual void SetVector(const int32_t& UniformId, const glm::vec3& Value) = 0;
        virtual void SetVector(const int32_t& UniformId, const glm::vec4& Value) = 0;

        virtual void SetVector(const int32_t& UniformId, const glm::ivec2& Value) = 0;
        virtual void SetVector(const int32_t& UniformId, const glm::ivec3& Value) = 0;
        virtual void SetVector(const int32_t& UniformId, const glm::ivec4& Value) = 0;

        virtual void SetMatrix(const int32_t& UniformId, const glm::mat4& Value) = 0;

        virtual int32_t GetTextureId(const String& TextureName) = 0;
        virtual void UseTexture(const int32_t& UniformId, Texture* TextureToUse) = 0;

        virtual void AddBinding(BufferBinding* Binding) = 0;

        virtual ~Shader() = default;
        // TODO Matrices
    };

    Shader* CompileShaderProgram(const String& VertexShaderSource, const String& FragementShaderSource);
} // namespace lucid::gpu
