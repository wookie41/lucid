#pragma once

#include "common/strings.hpp"
#include "common/math.hpp"

namespace lucid::gpu
{
    class Shader
    {
      public:
        virtual void Use() const = 0;
        virtual void Disable() const = 0;

        virtual void SetInt(const String& UniformName, const uint32_t& Value) const = 0;
        virtual void SetFloat(const String& UniformName, const float& Value) const = 0;
        virtual void SetBool(const String& UniformName, const bool& Value) const = 0;

        virtual void SetVector(const String& UniformName, const math::vec2& Value) const = 0;
        virtual void SetVector(const String& UniformName, const math::vec3& Value) const = 0;
        virtual void SetVector(const String& UniformName, const math::vec4& Value) const = 0;

        virtual void SetVector(const String& UniformName, const math::ivec2& Value) const = 0;
        virtual void SetVector(const String& UniformName, const math::ivec3& Value) const = 0;
        virtual void SetVector(const String& UniformName, const math::ivec4& Value) const = 0;

        // TODO Matrices
    };

    Shader* CompileShaderProgram(const String& VertexShaderSource, const String& FragementShaderSource);
} // namespace lucid::gpu
