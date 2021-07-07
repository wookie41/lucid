#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "common/types.hpp"

namespace lucid::math
{
    struct FRectangle
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

    inline real Lerp(const real& X, const real& Y, const real& T)
    {
        return X + ((Y - X) * T);
    }
    
    inline glm::vec3 Lerp(glm::vec3 x, glm::vec3 y, float t)
    {
        return x * (1.f - t) + y * t;
    }

    inline float Remap(const float& Value, const float& Low1, const float& High1, const float& Low2, const float& High2)
    {
        return Low2 + (Value - Low1) * (High2 - Low2) / (High1 - Low1);
    }

} // namespace lucid::misc
