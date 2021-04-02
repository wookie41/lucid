#pragma once

#include "common/types.hpp"
#include "schemas/types.hpp"

namespace lucid
{
    enum class EEngineInitError : u8
    {
        NO_ERROR,
        GPU_INIT_ERROR,
        
    };

    struct FEngineConfig
    {
        bool bHotReloadShaders;
    };
    
    EEngineInitError InitEngine(const FEngineConfig& InEngineConfig);
}
