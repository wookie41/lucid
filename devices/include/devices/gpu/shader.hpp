#pragma once

#include "common/strings.hpp"
#include "devices/gpu/buffer.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class CTexture;

    struct BufferBinding
    {
        CBuffer* BufferToUse = nullptr;
        EBufferBindPoint BindPoint = EBufferBindPoint::UNBOUND;
        int32_t Index = -1; // Used only for indexed buffers
    };

    class CShader : public CGPUObject
    {
      public:

        friend class ShaderManager;

        explicit CShader(const FANSIString& InName) : CGPUObject(InName) {}

        virtual void Use() = 0;
        virtual void Disable() = 0;

        virtual void SetupBuffersBindings() = 0;

        virtual u32 GetIdForUniform(const FANSIString& InUniformName) const = 0;

        virtual void SetInt(const FANSIString& InUniformName, const u32& Value) = 0;
        virtual void SetFloat(const FANSIString& InUniformName, const float& Value) = 0;
        virtual void SetBool(const FANSIString& InUniformName, const bool& Value) = 0;

        virtual void SetVector(const FANSIString& InUniformName, const glm::vec2& Value) = 0;
        virtual void SetVector(const FANSIString& InUniformName, const glm::vec3& Value) = 0;
        virtual void SetVector(const FANSIString& InUniformName, const glm::vec4& Value) = 0;

        virtual void SetVector(const FANSIString& InUniformName, const glm::ivec2& Value) = 0;
        virtual void SetVector(const FANSIString& InUniformName, const glm::ivec3& Value) = 0;
        virtual void SetVector(const FANSIString& InUniformName, const glm::ivec4& Value) = 0;

        virtual void SetMatrix(const FANSIString& InUniformName, const glm::mat4& Value) = 0;

        virtual void UseTexture(const FANSIString& InUniformName, CTexture* TextureToUse) = 0;
        virtual void RestoreTextureBindings() = 0;

        virtual void AddBinding(BufferBinding* Binding) = 0;

        virtual ~CShader() = default;
#ifndef NDEBUG
        /**
         * Used when hot-reloading shaders. The RecompiledShader is the shader program with updated shaders that we have to pull
         * to this shader, so the hot-reloading is transparent to the user and he can still use the same shader.
         */
        virtual void ReloadShader(CShader* RecompiledShader) = 0;
#endif
    };

    CShader* CompileShaderProgram(
        const FANSIString& InShaderName,
        const FANSIString& InVertexShaderSource,
        const FANSIString& InFragementShaderSource,
        const FANSIString& InGeometryShaderSource,
        const bool& InWarnMissingUniforms);

} // namespace lucid::gpu
