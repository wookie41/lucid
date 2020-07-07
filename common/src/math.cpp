#include "common/math.hpp"

namespace lucid::math
{
    mat4 CreateOrthographicProjectionMatrix(const float& Right,
                                            const float& Left,
                                            const float& Bottom,
                                            const float& Top,
                                            const float& Near,
                                            const float& Far)
    {
        return { { { 2.f / (Right - Left), 0, 0, 0 },
                   { 0, 2.f / (Top - Bottom), 0, 0 },
                   { 0, 0, -2.f / (Far - Near), 0 },
                   { -((Right + Left) / (Right - Left)), -(Top + Bottom) / (Top - Bottom),
                     -((Far + Near) / (Far - Near)), 1 } } };
    }
} // namespace lucid::math
