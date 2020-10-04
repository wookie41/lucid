#pragma once

#include "glm/glm.hpp"

namespace lucid::scene
{
    struct DirectionalLight
    {
        glm::vec3 Direction;
        glm::vec3 Color;
    };
}
