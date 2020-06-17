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

    struct TextureBinding : UniformVariable
    {
        GLint TextureIndex = 0;
        Texture* BoundTexture = nullptr;
    };

    class GLShader : public Shader
    {
      public:
        GLShader(const GLuint& GLShaderID,
                 StaticArray<UniformVariable> UniformVariables,
                 StaticArray<TextureBinding> TextureBindings);

        virtual void Use() override;
        virtual void Disable() override;

        virtual void SetInt(const String& UniformName, const uint32_t& Value) override;
        virtual void SetFloat(const String& UniformName, const float& Value) override;
        virtual void SetBool(const String& UniformName, const bool& Value) override;

        virtual void SetVector(const String& UniformName, const math::vec2& Value) override;
        virtual void SetVector(const String& UniformName, const math::vec3& Value) override;
        virtual void SetVector(const String& UniformName, const math::vec4& Value) override;

        virtual void SetVector(const String& UniformName, const math::ivec2& Value) override;
        virtual void SetVector(const String& UniformName, const math::ivec3& Value) override;
        virtual void SetVector(const String& UniformName, const math::ivec4& Value) override;

        virtual void SetMatrix(const String& UniformName, const math::mat4& Value) override;

        virtual void UseTexture(const String& TextureName, Texture* TextureToUse) override;

        virtual ~GLShader();

      private:
        GLint getUniformLocation(const String& Name) const;

        GLuint glShaderID;

        StaticArray<UniformVariable> uniformVariables;
        StaticArray<TextureBinding> textureBindings;
    };
} // namespace lucid::gpu