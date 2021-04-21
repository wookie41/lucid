#include "engine.hpp"

#include <ctime>

#include "stb_init.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/shaders_manager.hpp"

namespace lucid
{
    EEngineInitError InitEngine(const FEngineConfig& InEngineConfig)
    {
        srand(time(NULL));
        InitSTB();

        // @TODO Set default texture
        
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
