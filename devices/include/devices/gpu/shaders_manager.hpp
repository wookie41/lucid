#pragma once
#include "common/collections.hpp"
#include "common/strings.hpp"

namespace lucid::gpu
{
    class Shader;

    struct ShaderInstanceInfo
    {
        Shader* Shader;
        // @TODO Unicode paths support
        // @Note Paths are not freed, but it's okay, cause this code only hot-reloads shaders while in debug configuration
        DString VertexShaderPath;
        DString FragmentShaderPath;
        DString GeometryShaderPath;
    };

    class ShadersManager
    {
    public:

        // @TODO Unicode paths support
        Shader* CompileShader(const ANSIString& InShaderName,
                              const ANSIString& InVertexShaderPath,
                              const ANSIString& InFragementShaderPath,
                              const ANSIString& InGeometryShaderPath,
                              const bool& ShouldStoreShader = true);

#ifndef NDEBUG
        friend void ReloadShaders();
        void EnableHotReload();
        void DisableHotReload();
#endif

    private:

        Array<ShaderInstanceInfo> CompiledShaders { 8, true };

        const String BaseShadersPath { "shaders/glsl/base" };
    };

    extern ShadersManager GShadersManager;
}
