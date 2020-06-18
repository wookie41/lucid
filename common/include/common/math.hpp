#pragma once

#include <cstdint>

namespace lucid::math
{
#pragma pack(push, 1)

    struct vec2
    {
        union {
            float x = 0, r, u;
        };


        union {
            float y = 0, g, v;
        };

        vec2& operator+=(const vec2& Other);
    };

    struct vec3
    {
        union {
            float x = 0, r, u;
        };

        union {
            float y = 0, g, v;
        };

        union {
            float z = 0, b;
        };
    };

    struct vec4
    {
        union {
            float x = 0, r, u;
        };

        union {
            float y = 0, g, v;
        };

        union {
            float z = 0, b;
        };

        union {
            float w = 0, a;
        };
    };

    struct ivec2
    {
        union {
            int32_t x = 0, r, u;
        };


        union {
            int32_t y = 0, g, v;
        };
    };

    struct ivec3
    {
        union {
            int32_t x = 0, r, u;
        };

        union {
            int32_t y = 0, g, v;
        };

        union {
            int32_t z = 0, b;
        };
    };


    struct ivec4
    {
        union {
            int32_t x = 0, r, u;
        };

        union {
            int32_t y = 0, g, v;
        };

        union {
            int32_t z = 0, b;
        };

        union {
            int32_t w = 0, a;
        };
    };

    struct mat4
    {
        float Cols[4][4];
    };

    //TODO ViewMatrix

    mat4 CreateOrthographicProjectionMatrix(const float& Right,
                                            const float& Left,
                                            const float& Bottom,
                                            const float& Top,
                                            const float& Near,
                                            const float& Far);

#pragma pack(pop)
} // namespace lucid::math