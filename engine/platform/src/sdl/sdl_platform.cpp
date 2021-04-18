#include <SDL_video.h>
#include <glm/vec2.hpp>

#include "platform/platform.hpp"

namespace lucid::platform
{
    void Update()
    {
        _UpdateSystem();
    }

    glm::ivec2 GetDisplaySizeInPixels(const u8& DisplayIndex)
    {
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(DisplayIndex, &DM);
        return { DM.w, DM.h };
    }
} // namespace lucid::platform