#pragma once
#include "common/types.hpp"

namespace lucid::scene
{
    enum ESpaceType : i32
    {
        WORLD_SPACE = 0,
        VIEW_SPACE  = 1,
        CLIP_SPACE  = 2
    };
}
