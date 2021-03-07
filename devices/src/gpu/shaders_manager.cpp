#include "devices/gpu/shaders_manager.hpp"

#include <stb_ds.h>


#include "common/log.hpp"
#include "devices/gpu/shader.hpp"
#include "platform/fs.hpp"
#include "platform/platform.hpp"

namespace lucid::gpu
{
    ShadersManager GShadersManager;

    void ReloadShaders();

    Shader* ShadersManager::CompileShader(const ANSIString& ShaderName,
                                          const ANSIString& VertexShaderPath,
                                          const ANSIString& FragmentShaderPath,
                                          const ANSIString& GeometryShaderPath,
                                          const bool& ShouldStoreShader)
    {
        DString VertexShaderSource = platform::ReadFile(VertexShaderPath, true);
        DString FragmentShaderSource = platform::ReadFile(FragmentShaderPath, true);

        DString GeometryShaderSource{""};
        if (GeometryShaderPath.GetLength())
        {
            GeometryShaderSource = platform::ReadFile(GeometryShaderPath, true);
        }

        if (VertexShaderSource.GetLength() == 0)
        {
            LUCID_LOG(LogLevel::WARN, "Failed to read vertex shader source from file %s while compiling shader '%s'",
                      *VertexShaderPath, *ShaderName);
            return nullptr;
        }

        if (FragmentShaderSource.GetLength() == 0)
        {
            LUCID_LOG(LogLevel::WARN, "Failed to read fragment shader source from file %s while compiling shader '%s'",
                      *FragmentShaderPath, *ShaderName);
            VertexShaderSource.Free();
            return nullptr;
        }

        if (GeometryShaderPath.GetLength() > 0 && GeometryShaderSource.GetLength() == 0)
        {
            LUCID_LOG(LogLevel::WARN, "Failed to read geometry shader source from file %s while compiling shader '%s'",
                      *GeometryShaderPath, *ShaderName);
            VertexShaderSource.Free();
            FragmentShaderSource.Free();
            return nullptr;
        }

        Shader* CompiledShader = gpu::CompileShaderProgram(
            ShaderName,
            VertexShaderSource,
            FragmentShaderSource,
            GeometryShaderSource
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
            const ShaderInstanceInfo ShaderInstanceInfo {
                CompiledShader,
                CopyToString(*VertexShaderPath, VertexShaderPath.GetLength()),
                CopyToString(*FragmentShaderPath, FragmentShaderPath.GetLength()),
                CopyToString(*GeometryShaderPath, GeometryShaderPath.GetLength())
            };
            CompiledShaders.Add(ShaderInstanceInfo);
        }

        return CompiledShader;
    }

    void ShadersManager::EnableHotReload()
    {
        platform::AddDirectoryListener(BaseShadersPath, ReloadShaders);
    }

    void ShadersManager::DisableHotReload()
    {
        platform::RemoveDirectoryListener(BaseShadersPath, ReloadShaders);
    }

    void ReloadShaders()
    {
        platform::ExecuteCommand(String { LUCID_TEXT("sh tools\\preprocess_shaders.sh") });

        for (u32 i = 0; i < GShadersManager.CompiledShaders.GetLength(); ++i)
        {
            const ShaderInstanceInfo* ShaderInfo = GShadersManager.CompiledShaders[i];
            Shader* RecompiledShader = GShadersManager.CompileShader(ShaderInfo->Shader->GetName(),
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
