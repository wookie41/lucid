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
        GLShader(const GLuint& GLShaderID,
                 StaticArray<UniformVariable> UniformVariables,
                 StaticArray<TextureBinding> TextureBindings);

        virtual void Use() override;

        virtual void SetupBuffersBindings() override;

        virtual void Disable() override;

        virtual int32_t GetIdForUniform(const String& UniformName) const override;

        virtual void SetInt(const int32_t& UniformId, const uint32_t& Value) override;
        virtual void SetFloat(const int32_t& UniformId, const float& Value) override;
        virtual void SetBool(const int32_t& UniformId, const bool& Value) override;

        virtual void SetVector(const int32_t& UniformId, const glm::vec2& Value) override;
        virtual void SetVector(const int32_t& UniformId, const glm::vec3& Value) override;
        virtual void SetVector(const int32_t& UniformId, const glm::vec4& Value) override;

        virtual void SetVector(const int32_t& UniformId, const glm::ivec2& Value) override;
        virtual void SetVector(const int32_t& UniformId, const glm::ivec3& Value) override;
        virtual void SetVector(const int32_t& UniformId, const glm::ivec4& Value) override;

        virtual void SetMatrix(const int32_t& UniformId, const glm::mat4& Value) override;

        virtual int32_t GetTextureId(const String& TextureName) override;
        virtual void UseTexture(const int32_t& TextureId, Texture* TextureToUse) override;

        virtual void AddBinding(BufferBinding* Binding) override;

        virtual ~GLShader();

      private:
        GLint getUniformLocation(const String& Name) const;

        GLuint glShaderID;

        StaticArray<UniformVariable> uniformVariables;
        StaticArray<TextureBinding> textureBindings;
        LinkedList<BufferBinding> buffersBindings;
    };
} // namespace lucid::gpu