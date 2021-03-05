#include "devices/gpu/shader.hpp"

#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class Buffer;

    struct UniformVariable
    {
        GLint Location = 0;
        DString Name { "" };
        Type Type = Type::UNSUPPORTED;
    };

    struct TextureBinding : UniformVariable
    {
        GLint TextureIndex = 0;
        Texture* BoundTexture = nullptr;
    };

    class GLShader : public Shader
    {
      public:
        GLShader(const ANSIString& InName,
                 const GLuint& GLShaderID,
                 Array<UniformVariable> UniformVariables,
                 Array<TextureBinding> TextureBindings,
                 const bool& WarnMissingUniforms);

        virtual void Use() override;

        virtual void SetupBuffersBindings() override;

        virtual void Disable() override;

        virtual void SetInt(const ANSIString& InUniformName, const u32& Value) override;
        virtual void SetFloat(const ANSIString& InUniformName, const float& Value) override;
        virtual void SetBool(const  ANSIString& InUniformName, const bool& Value) override;

        virtual void SetVector(const ANSIString& InUniformName, const glm::vec2& Value) override;
        virtual void SetVector(const ANSIString& InUniformName, const glm::vec3& Value) override;
        virtual void SetVector(const ANSIString& InUniformName, const glm::vec4& Value) override;

        virtual void SetVector(const ANSIString& InUniformName, const glm::ivec2& Value) override;
        virtual void SetVector(const ANSIString& InUniformName, const glm::ivec3& Value) override;
        virtual void SetVector(const ANSIString& InUniformName, const glm::ivec4& Value) override;

        virtual void SetMatrix(const ANSIString& InUniformName, const glm::mat4& Value) override;

        virtual void UseTexture(const ANSIString& InUniformName, Texture* TextureToUse) override;
        
        virtual void RestoreTextureBindings() override;
        
        virtual void AddBinding(BufferBinding* Binding) override;

#ifndef NDEBUG
        void ReloadShader(Shader* RecompiledShader) override;
#endif
        u32 GetIdForUniform(const ANSIString& InUniformName) const;
        u32 GetTextureId(const ANSIString& InUniformName) const;

        virtual void Free() override;
        
        virtual ~GLShader() = default;

      private:

        GLuint glShaderID;

        Array<UniformVariable> uniformVariables;
        Array<TextureBinding> textureBindings;
        LinkedList<BufferBinding> buffersBindings;
        bool warnMissingUniforms;
    };
} // namespace lucid::gpu