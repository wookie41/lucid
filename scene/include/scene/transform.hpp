#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace lucid::scene
{
    struct Transform3D
    {
        glm::vec3 Translation{ 0, 0, 0 };
        glm::quat Rotation{ 0, 0, 1, 0 };
        glm::vec3 Scale{ 1, 1, 1 };
    };
} // namespace lucid::scene
