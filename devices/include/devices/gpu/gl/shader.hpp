#include "devices/gpu/shader.hpp"

#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class Buffer;

    struct UniformVariable
    {
        GLint GLIndex = 0;
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
        GLShader(const String& InName,
                 const GLuint& GLShaderID,
                 StaticArray<UniformVariable> UniformVariables,
                 StaticArray<TextureBinding> TextureBindings,
                 const bool& WarnMissingUniforms);

        virtual void Use() override;

        virtual void SetupBuffersBindings() override;

        virtual void Disable() override;

        virtual void SetInt(const String& Name, const uint32_t& Value) override;
        virtual void SetFloat(const String& Name, const float& Value) override;
        virtual void SetBool(const String& Name, const bool& Value) override;

        virtual void SetVector(const String& Name, const glm::vec2& Value) override;
        virtual void SetVector(const String& Name, const glm::vec3& Value) override;
        virtual void SetVector(const String& Name, const glm::vec4& Value) override;

        virtual void SetVector(const String& Name, const glm::ivec2& Value) override;
        virtual void SetVector(const String& Name, const glm::ivec3& Value) override;
        virtual void SetVector(const String& Name, const glm::ivec4& Value) override;

        virtual void SetMatrix(const String& Name, const glm::mat4& Value) override;

        virtual void UseTexture(const String& Name, Texture* TextureToUse) override;

        virtual void AddBinding(BufferBinding* Binding) override;

        int32_t GetIdForUniform(const String& Name) const;
        int32_t GetTextureId(const String& Name) const;

        virtual ~GLShader();

      private:
        GLint getUniformLocation(const String& Name) const;

        GLuint glShaderID;

        StaticArray<UniformVariable> uniformVariables;
        StaticArray<TextureBinding> textureBindings;
        LinkedList<BufferBinding> buffersBindings;
        bool warnMissingUniforms;
    };
} // namespace lucid::gpu