#include "devices/gpu/shaders_manager.hpp"

#include <engine/engine.hpp>


#include "common/log.hpp"
#include "devices/gpu/shader.hpp"
#include "platform/fs.hpp"
#include "platform/platform.hpp"
namespace lucid::gpu
{
    void ReloadShaders();

    CShader* CShadersManager::CompileShader(const FShaderInfo& ShaderInfo, const bool& ShouldStoreShader)
    {
        FDString VertexShaderSource = platform::ReadFile(*ShaderInfo.VertexShaderSourcePath, true);
        FDString FragmentShaderSource = platform::ReadFile(*ShaderInfo.FragmentShaderSourcePath, true);

        FDString GeometryShaderSource {""};
        if (ShaderInfo.GeometryShaderSourcePath.GetLength())
        {
            GeometryShaderSource = platform::ReadFile(*ShaderInfo.GeometryShaderSourcePath, true);
        }

        if (VertexShaderSource.GetLength() == 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to read vertex shader source from file %s while compiling shader '%s'",
                      *ShaderInfo.VertexShaderSourcePath, *ShaderInfo.Name);
            return nullptr;
        }

        if (FragmentShaderSource.GetLength() == 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to read fragment shader source from file %s while compiling shader '%s'",
                      *ShaderInfo.FragmentShaderSourcePath, *ShaderInfo.Name);
            VertexShaderSource.Free();
            return nullptr;
        }

        if (ShaderInfo.GeometryShaderSourcePath.GetLength() > 0 && GeometryShaderSource.GetLength() == 0)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to read geometry shader source from file %s while compiling shader '%s'",
                      *ShaderInfo.GeometryShaderSourcePath, *ShaderInfo.Name);
            VertexShaderSource.Free();
            FragmentShaderSource.Free();
            return nullptr;
        }

        CShader* CompiledShader = gpu::CompileShaderProgram(
            ShaderInfo.Name,
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
            ShaderInfoByName.Add(*ShaderInfo.Name, ShaderInfo);
            CompiledShadersByName.Add(*ShaderInfo.Name, CompiledShader);            
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
        platform::ExecuteCommand(FSString { LUCID_TEXT("sh tools\\scripts\\preprocess_shaders.sh") });

        for (u64 i = 0; i < GEngine.GetShadersManager().ShaderInfoByName.GetLength(); ++i)
        {
            const FShaderInfo& ShaderInfo = GEngine.GetShadersManager().ShaderInfoByName.Get(i);
            CShader* ShaderToReload = GEngine.GetShadersManager().CompiledShadersByName.Get(i);
            
            CShader* RecompiledShader = GEngine.GetShadersManager().CompileShader( ShaderInfo, false);
            if (RecompiledShader)
            {
                ShaderToReload->ReloadShader(RecompiledShader);
                delete RecompiledShader;
            }
        }
    }

    void CShadersManager::LoadShadersDatabase(const FShadersDataBase& InShadersDatabase)
    {
        for (const FShaderInfo& ShaderInfo : InShadersDatabase.Shaders)
        {
            if(CompileShader(ShaderInfo, true))
            {
                LUCID_LOG(ELogLevel::INFO, "Loaded shader %s", *ShaderInfo.Name);
            }
            else
            {
                LUCID_LOG(ELogLevel::INFO, "Failed to load shader %s", *ShaderInfo.Name);
            }
        }
    }

    CShader* CShadersManager::GetShaderByName(const FString& ShaderName)
    {
        CShader* Shader = CompiledShadersByName.Get(*ShaderName);
        if (Shader)
        {
            return Shader;;
        }
        LUCID_LOG(ELogLevel::WARN, "Shader %s not found", *ShaderName);
    }

} // namespace lucid::gpu