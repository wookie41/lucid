#pragma once

#include "devices/gpu/shader.hpp"

#include "glad/glad.h"
#include "common/collections.hpp"

namespace lucid::gpu
{
    class CGPUBuffer;

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
                 const FString& InName);

        void SetObjectName() override;
        
        virtual void Use() override;

        virtual void SetupBuffersBindings() override;

        virtual void Disable() override;

        virtual void SetInt(const FString& InUniformName, const i32& Value) override;
        virtual void SetUInt(const FString& InUniformName, const u32& Value) override;
        virtual void SetFloat(const FString& InUniformName, const float& Value) override;
        virtual void SetBool(const  FString& InUniformName, const bool& Value) override;

        virtual void SetVector(const FString& InUniformName, const glm::vec2& Value) override;
        virtual void SetVector(const FString& InUniformName, const glm::vec3& Value) override;
        virtual void SetVector(const FString& InUniformName, const glm::vec4& Value) override;

        virtual void SetVector(const FString& InUniformName, const glm::ivec2& Value) override;
        virtual void SetVector(const FString& InUniformName, const glm::ivec3& Value) override;
        virtual void SetVector(const FString& InUniformName, const glm::ivec4& Value) override;

        virtual void SetMatrix(const FString& InUniformName, const glm::mat4& Value) override;

        virtual void UseTexture(const FString& InUniformName, CTexture* TextureToUse) override;
        virtual void UseBindlessTexture(const FString& InUniformName, const u64& Value) override;

        virtual void RestoreTextureBindings() override;
        
        virtual void AddBinding(BufferBinding* Binding) override;

#ifndef NDEBUG
        void ReloadShader(CShader* RecompiledShader) override;
#endif
        u32 GetIdForUniform(const FString& InUniformName) const;
        u32 GetTextureId(const FString& InUniformName) const;

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