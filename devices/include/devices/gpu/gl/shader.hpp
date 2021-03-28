#pragma once

#include "devices/gpu/shader.hpp"

#include "GL/glew.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class CBuffer;

    struct FUniformVariable
    {
        GLint Location = 0;
        FDString Name { "" };
        EType Type = EType::UNSUPPORTED;
    };

    struct FTextureBinding : FUniformVariable
    {
        GLint TextureIndex = 0;
        CTexture* BoundTexture = nullptr;
    };

    class CGLShader : public CShader
    {
      public:
        CGLShader(const GLuint& GLShaderID,
                 FArray<FUniformVariable> UniformVariables,
                 FArray<FTextureBinding> TextureBindings,
                 const bool& WarnMissingUniforms,
                 const FANSIString& InName);

        void SetObjectName() override;
        
        virtual void Use() override;

        virtual void SetupBuffersBindings() override;

        virtual void Disable() override;

        virtual void SetInt(const FANSIString& InUniformName, const u32& Value) override;
        virtual void SetFloat(const FANSIString& InUniformName, const float& Value) override;
        virtual void SetBool(const  FANSIString& InUniformName, const bool& Value) override;

        virtual void SetVector(const FANSIString& InUniformName, const glm::vec2& Value) override;
        virtual void SetVector(const FANSIString& InUniformName, const glm::vec3& Value) override;
        virtual void SetVector(const FANSIString& InUniformName, const glm::vec4& Value) override;

        virtual void SetVector(const FANSIString& InUniformName, const glm::ivec2& Value) override;
        virtual void SetVector(const FANSIString& InUniformName, const glm::ivec3& Value) override;
        virtual void SetVector(const FANSIString& InUniformName, const glm::ivec4& Value) override;

        virtual void SetMatrix(const FANSIString& InUniformName, const glm::mat4& Value) override;

        virtual void UseTexture(const FANSIString& InUniformName, CTexture* TextureToUse) override;
        
        virtual void RestoreTextureBindings() override;
        
        virtual void AddBinding(BufferBinding* Binding) override;

#ifndef NDEBUG
        void ReloadShader(CShader* RecompiledShader) override;
#endif
        u32 GetIdForUniform(const FANSIString& InUniformName) const;
        u32 GetTextureId(const FANSIString& InUniformName) const;

        virtual void Free() override;
        
        virtual ~CGLShader() = default;

      private:

        GLuint glShaderID;

        FArray<FUniformVariable> uniformVariables;
        FArray<FTextureBinding> TextureBindings;
        FLinkedList<BufferBinding> buffersBindings;
        bool warnMissingUniforms;
    };
} // namespace lucid::gpu