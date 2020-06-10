#include "devices/gpu/shader.hpp"

#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class Buffer;

    struct UniformVariable
    {
        GLint GLIndex;
        String Name;
    };

    class GLShader : public Shader
    {
      public:
        GLShader(const GLuint& GLShaderID,
                 StaticArray<UniformVariable> UniformVariables,
                 StaticArray<UniformVariable> SamplerUniforms);

        virtual void Use() const override;
        virtual void Disable() const override;

        virtual void SetInt(const String& UniformName, const uint32_t& Value) const override;
        virtual void SetFloat(const String& UniformName, const float& Value) const override;
        virtual void SetBool(const String& UniformName, const bool& Value) const override;

        virtual void SetVector(const String& UniformName, const math::vec2& Value) const override;
        virtual void SetVector(const String& UniformName, const math::vec3& Value) const override;
        virtual void SetVector(const String& UniformName, const math::vec4& Value) const override;

        virtual void SetVector(const String& UniformName, const math::ivec2& Value) const override;
        virtual void SetVector(const String& UniformName, const math::ivec3& Value) const override;
        virtual void SetVector(const String& UniformName, const math::ivec4& Value) const override;

      private:

        GLint getUniformLocation(const String& Name, const bool& IsSampler) const;

        GLuint glShaderID;

        StaticArray<UniformVariable> uniformVariables;
        StaticArray<UniformVariable> samplerUniforms;
    };
} // namespace lucid::gpu