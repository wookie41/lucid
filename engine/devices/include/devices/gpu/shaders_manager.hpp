﻿#pragma once

#include "common/strings.hpp"
#include "common/collections.hpp"
#include "schemas/types.hpp"

namespace lucid::gpu
{
    class CShader;
    struct FGPUState;

    class CShadersManager
    {
    public: 

        void LoadShadersDatabase(const FShadersDataBase& InShadersDatabase);

        CShader*                            GetShaderByName(const FString& ShaderName);
        inline const FStringHashMap<CShader*>& GetAllShaders() const { return CompiledShadersByName; }
        
#ifndef NDEBUG
        friend void ReloadShaders();
        void EnableHotReload();
        void DisableHotReload();
#endif

    private:
        
        CShader* CompileShader(const FShaderInfo& ShaderInfo, const bool& ShouldStoreShader);
        
        FStringHashMap<FShaderInfo>    ShaderInfoByName;
        FStringHashMap<CShader*>       CompiledShadersByName;

        const FSString BaseShadersPath { "shaders/glsl/base" };
    };
#ifndef NDEBUG
    void ReloadShaders();
#endif

}
