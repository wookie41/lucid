#pragma once
#include "common/collections.hpp"
#include "common/strings.hpp"

namespace lucid::gpu
{
    class CShader;
    struct FGPUState;
    struct FShaderInstanceInfo
    {
        CShader* Shader;
        // @TODO Unicode paths support
        // @Note Paths are not freed, but it's okay, cause this code only hot-reloads shaders while in debug configuration
        FDString VertexShaderPath;
        FDString FragmentShaderPath;
        FDString GeometryShaderPath;
    };

    class CShadersManager
    {
    public:

        // @TODO Unicode paths support
        CShader* CompileShader(const FString& InShaderName,
                              const FString& InVertexShaderPath,
                              const FString& InFragementShaderPath,
                              const FString& InGeometryShaderPath,
                              const bool& ShouldStoreShader = true);

#ifndef NDEBUG
        friend void ReloadShaders();
        void EnableHotReload();
        void DisableHotReload();
#endif

    private:

        FArray<FShaderInstanceInfo> CompiledShaders { 8, true };

        const FSString BaseShadersPath { "shaders/glsl/base" };
    };

    extern CShadersManager GShadersManager;
}
