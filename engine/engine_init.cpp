#include "engine_init.hpp"

#include <ctime>

#include "stb_init.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/shaders_manager.hpp"
#include "resources/texture.hpp"

namespace lucid
{
    EEngineInitError InitEngine(const FEngineConfig& InEngineConfig)
    {
        srand(time(NULL));
        InitSTB();

        if (gpu::Init({}) < 0)
        {
            return EEngineInitError::GPU_INIT_ERROR;
        }
        
        if (InEngineConfig.bHotReloadShaders)
        {
            gpu::GShadersManager.EnableHotReload();
        }
        
    }
}
