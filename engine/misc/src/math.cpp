#include "misc/math.hpp"
#include <random>

namespace lucid::math
{
    float RandomFloat()
    {
        static std::default_random_engine generator;
        static std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
        return randomFloats(generator);
    }

    glm::vec2 RandomVec2()
    {
        return { RandomFloat(), RandomFloat() };
    }

    glm::vec3 RandomVec3()
    {
        return { RandomFloat(), RandomFloat(), RandomFloat() };
    }

    real Lerp(const real& X, const real& Y, const real& T);
}
