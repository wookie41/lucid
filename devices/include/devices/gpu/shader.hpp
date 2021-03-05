#pragma once

#include "common/strings.hpp"
#include "devices/gpu/buffer.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class Texture;

    struct BufferBinding
    {
        Buffer* BufferToUse = nullptr;
        BufferBindPoint BindPoint = BufferBindPoint::UNBOUND;
        int32_t Index = -1; // Used only for indexed buffers
    };

    class Shader
    {
      public:

        friend class ShaderManager;

        explicit Shader(const ANSIString& InName);

        virtual void Use() = 0;
        virtual void Disable() = 0;

        virtual void SetupBuffersBindings() = 0;

        virtual u32 GetIdForUniform(const ANSIString& InUniformName) const = 0;

        virtual void SetInt(const ANSIString& InUniformName, const u32& Value) = 0;
        virtual void SetFloat(const ANSIString& InUniformName, const float& Value) = 0;
        virtual void SetBool(const ANSIString& InUniformName, const bool& Value) = 0;

        virtual void SetVector(const ANSIString& InUniformName, const glm::vec2& Value) = 0;
        virtual void SetVector(const ANSIString& InUniformName, const glm::vec3& Value) = 0;
        virtual void SetVector(const ANSIString& InUniformName, const glm::vec4& Value) = 0;

        virtual void SetVector(const ANSIString& InUniformName, const glm::ivec2& Value) = 0;
        virtual void SetVector(const ANSIString& InUniformName, const glm::ivec3& Value) = 0;
        virtual void SetVector(const ANSIString& InUniformName, const glm::ivec4& Value) = 0;

        virtual void SetMatrix(const ANSIString& InUniformName, const glm::mat4& Value) = 0;

        virtual void UseTexture(const ANSIString& InUniformName, Texture* TextureToUse) = 0;
        virtual void RestoreTextureBindings() = 0;

        virtual void AddBinding(BufferBinding* Binding) = 0;

        virtual void Free() = 0;

        virtual ~Shader() = default;
        
        inline const ANSIString& GetName() const { return ShaderName; }

#ifndef NDEBUG
        /**
         * Used when hot-reloading shaders. The RecompiledShader is the shader program with updated shaders that we have to pull
         * to this shader, so the hot-reloading is transparent to the user and he can still use the same shader.
         */
        virtual void ReloadShader(Shader* RecompiledShader) = 0;
#endif
    protected:
        const ANSIString& ShaderName;
    };

    Shader* CompileShaderProgram(const ANSIString& InShaderName,
                                 const ANSIString& InVertexShaderSource,
                                 const ANSIString& InFragementShaderSource,
                                 const ANSIString& InGeometryShaderSource,
                                 const bool& InWarnMissingUniforms = false);
} // namespace lucid::gpu
