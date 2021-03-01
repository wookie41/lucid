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
        int32_t Index = -1;

        bool IsValid = false;
    };

    class Shader
    {
      public:

        friend class ShaderManager;

        explicit Shader(const String& InName);

        virtual void Use() = 0;
        virtual void Disable() = 0;

        virtual void SetupBuffersBindings() = 0;

        // Return -1 if uniform doesn't exist
        virtual u32 GetIdForUniform(const String& Name) const = 0;

        virtual void SetInt(const String& Name, const u32& Value) = 0;
        virtual void SetFloat(const String& Name, const float& Value) = 0;
        virtual void SetBool(const String& Name, const bool& Value) = 0;

        virtual void SetVector(const String& Name, const glm::vec2& Value) = 0;
        virtual void SetVector(const String& Name, const glm::vec3& Value) = 0;
        virtual void SetVector(const String& Name, const glm::vec4& Value) = 0;

        virtual void SetVector(const String& Name, const glm::ivec2& Value) = 0;
        virtual void SetVector(const String& Name, const glm::ivec3& Value) = 0;
        virtual void SetVector(const String& Name, const glm::ivec4& Value) = 0;

        virtual void SetMatrix(const String& Name, const glm::mat4& Value) = 0;

        virtual void UseTexture(const String& Name, Texture* TextureToUse) = 0;
        virtual void RestoreTextureBindings() = 0;

        virtual void AddBinding(BufferBinding* Binding) = 0;

        virtual ~Shader() = default;
        
        inline const String& GetName() const { return ShaderName; }

#ifndef NDEBUG
        /**
         * Used when hot-reloading shaders. The RecompiledShader is the shader program with updated shaders that we have to pull
         * to this shader, so the hot-reloading is transparent to the user and he can still use the same shader.
         */
        virtual void ReloadShader(Shader* RecompiledShader) = 0;
#endif
    protected:
        const String ShaderName;
    };

    Shader* CompileShaderProgram(const String& ShaderName,
                                 const char* VertexShaderSource,
                                 const char* FragementShaderSource,
                                 const char* GeometryShaderSource,
                                 const bool& WarnMissingUniforms = false);
} // namespace lucid::gpu
