#include "platform/util.hpp"

#include "SDL2/SDL.h"

namespace lucid::platform
{
    real SimulationStep = 1.0/60.0;

    real GetCurrentTimeSeconds()
    {
        return static_cast<real>(SDL_GetTicks()) / static_cast<real>(1000);
    }
}