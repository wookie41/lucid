#include "devices/gpu/shaders_manager.hpp"

#include <stb_ds.h>


#include "common/log.hpp"
#include "devices/gpu/shader.hpp"
#include "platform/fs.hpp"
#include "platform/platform.hpp"

namespace lucid::gpu
{
    CShadersManager GShadersManager;

    void ReloadShaders();

    CShader* CShadersManager::CompileShader(const FANSIString& ShaderName,
                                          const FANSIString& VertexShaderPath,
                                          const FANSIString& FragmentShaderPath,
                                          const FANSIString& GeometryShaderPath,
                                          const bool& ShouldStoreShader)
    {
        FDString VertexShaderSource = platform::ReadFile(VertexShaderPath, true);
        FDString FragmentShaderSource = platform::ReadFile(FragmentShaderPath, true);

        FDString GeometryShaderSource{""};
        if (GeometryShaderPath.GetLength())
        {
            GeometryShaderSource = platform::ReadFile(GeometryShaderPath, true);
        }

        if (VertexShaderSource.GetLength() == 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to read vertex shader source from file %s while compiling shader '%s'",
                      *VertexShaderPath, *ShaderName);
            return nullptr;
        }

        if (FragmentShaderSource.GetLength() == 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to read fragment shader source from file %s while compiling shader '%s'",
                      *FragmentShaderPath, *ShaderName);
            VertexShaderSource.Free();
            return nullptr;
        }

        if (GeometryShaderPath.GetLength() > 0 && GeometryShaderSource.GetLength() == 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to read geometry shader source from file %s while compiling shader '%s'",
                      *GeometryShaderPath, *ShaderName);
            VertexShaderSource.Free();
            FragmentShaderSource.Free();
            return nullptr;
        }

        CShader* CompiledShader = gpu::CompileShaderProgram(
            ShaderName,
            VertexShaderSource,
            FragmentShaderSource,
            GeometryShaderSource,
            false
        );
        
        if (CompiledShader == nullptr)
        {
            VertexShaderSource.Free();
            FragmentShaderSource.Free();
            GeometryShaderSource.Free();
            return nullptr;
        }

        if (ShouldStoreShader)
        {
            const FShaderInstanceInfo ShaderInstanceInfo {
                CompiledShader,
                CopyToString(*VertexShaderPath, VertexShaderPath.GetLength()),
                CopyToString(*FragmentShaderPath, FragmentShaderPath.GetLength()),
                CopyToString(*GeometryShaderPath, GeometryShaderPath.GetLength())
            };
            CompiledShaders.Add(ShaderInstanceInfo);
        }

        return CompiledShader;
    }

    void CShadersManager::EnableHotReload()
    {
        platform::AddDirectoryListener(BaseShadersPath, ReloadShaders);
    }

    void CShadersManager::DisableHotReload()
    {
        platform::RemoveDirectoryListener(BaseShadersPath, ReloadShaders);
    }

    void ReloadShaders()
    {
        platform::ExecuteCommand(FString { LUCID_TEXT("sh tools\\scripts\\preprocess_shaders.sh") });

        for (u32 i = 0; i < GShadersManager.CompiledShaders.GetLength(); ++i)
        {
            const FShaderInstanceInfo* ShaderInfo = GShadersManager.CompiledShaders[i];
            
            CShader* RecompiledShader = GShadersManager.CompileShader( ShaderInfo->Shader->GetName() ,
                                                                     ShaderInfo->VertexShaderPath,
                                                                     ShaderInfo->FragmentShaderPath,
                                                                     ShaderInfo->GeometryShaderPath, false);
            if (RecompiledShader)
            {
                ShaderInfo->Shader->ReloadShader(RecompiledShader);
                delete RecompiledShader;
            }
        }
    }
} // namespace lucid::gpu
