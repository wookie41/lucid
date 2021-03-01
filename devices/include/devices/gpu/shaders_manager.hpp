#pragma once
#include "common/strings.hpp"

namespace lucid::gpu
{
    class Shader;

    struct ShaderInstanceInfo
    {
        Shader* Shader;
        String VertexShaderPath;
        String FragmentShaderPath;
        String GeometryShaderPath;
    };

    class ShadersManager
    {
    public:

        Shader* CompileShader(const String& ShaderName,
                              const String& VertexShaderPath,
                              const String& FragementShaderPath,
                              const String& GeometryShaderPath,
                              const bool& ShouldStoreShader = true);

#ifndef NDEBUG
        friend void ReloadShaders();
        void EnableHotReload();
        void DisableHotReload();
#endif

    private:
        ShaderInstanceInfo* CompiledShaders = nullptr;
    };

    extern ShadersManager GShadersManager;
}
