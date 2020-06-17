#pragma once

#include "common/strings.hpp"
#include "common/math.hpp"

namespace lucid::gpu
{
    class Texture;

    class Shader
    {
      public:
        virtual void Use() = 0;
        virtual void Disable() = 0;

        virtual void SetInt(const String& UniformName, const uint32_t& Value) = 0;
        virtual void SetFloat(const String& UniformName, const float& Value) = 0;
        virtual void SetBool(const String& UniformName, const bool& Value) = 0;

        virtual void SetVector(const String& UniformName, const math::vec2& Value) = 0;
        virtual void SetVector(const String& UniformName, const math::vec3& Value) = 0;
        virtual void SetVector(const String& UniformName, const math::vec4& Value) = 0;

        virtual void SetVector(const String& UniformName, const math::ivec2& Value) = 0;
        virtual void SetVector(const String& UniformName, const math::ivec3& Value) = 0;
        virtual void SetVector(const String& UniformName, const math::ivec4& Value) = 0;
        
        virtual void SetMatrix(const String& UniformName, const math::mat4& Value) = 0;

        virtual void UseTexture(const String& TextureName, Texture* TextureToUse) = 0;

        virtual ~Shader() = default;
        // TODO Matrices
    };

    Shader* CompileShaderProgram(const String& VertexShaderSource, const String& FragementShaderSource);
} // namespace lucid::gpu
