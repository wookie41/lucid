#pragma once

#include "common/types.hpp"

namespace lucid
{
    enum class EEngineInitError : u8
    {
        NONE,
        GPU_INIT_ERROR,
    };

    struct FEngineConfig
    {
        bool bHotReloadShaders;
    };
    
    EEngineInitError InitEngine(const FEngineConfig& InEngineConfig);
}
