#include "common/math.hpp"

namespace lucid::math
{
    vec2& vec2::operator+=(const vec2& Other)
    {
        x += Other.x;
        y += Other.y;

        return *this;
    }

    mat4 CreateOrthographicProjectionMatrix(const float& Right,
                                            const float& Left,
                                            const float& Bottom,
                                            const float& Top,
                                            const float& Far,
                                            const float& Near)
    {
        return 
    {{
        { 2.f / (Right - Left), 0, 0, 0 },
        { 0, 1.f / Top, 0, 0 },
        { 0, 0, -2.f / (Far - Near), -((Far + Near) / (Far - Near)) },
        { 0, 0, 0, 1 },    
    }};
    }
} // namespace lucid::math
