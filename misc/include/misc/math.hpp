#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace lucid::misc
{
    struct IRectangle
    {
        int X, Y;
        int Width, Height;
    };

    // Returns a random float in range <0; 1>
    float RandomFloat();

    // Returns a random vec2 with components in range <0; 1>
    glm::vec2 RandomVec2();

    // Returns a random vec3 with components in range <0; 1>
    glm::vec3 RandomVec3();
} // namespace lucid::misc
