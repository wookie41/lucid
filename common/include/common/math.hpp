#pragma once

#include <cstdint>

namespace lucid::math
{
#pragma pack(push, 1)

    struct vec2
    {
        union {
            float x, r, u;
        };


        union {
            float y, g, v;
        };
    };

    struct vec3
    {
        union {
            float x, r, u;
        };

        union {
            float y, g, v;
        };

        union {
            float z, b;
        };
    };

    struct vec4
    {
        union {
            float x, r, u;
        };

        union {
            float y, g, v;
        };

        union {
            float z, b;
        };

        union {
            float w, a;
        };
    };

    struct ivec2
    {
        union {
            int32_t x, r, u;
        };


        union {
            int32_t y, g, v;
        };
    };

    struct ivec3
    {
        union {
            int32_t x, r, u;
        };

        union {
            int32_t y, g, v;
        };

        union {
            int32_t z, b;
        };
    };


    struct ivec4
    {
        union {
            int32_t x, r, u;
        };

        union {
            int32_t y, g, v;
        };

        union {
            int32_t z, b;
        };

        union {
            int32_t w, a;
        };
    };
#pragma pack(pop)
} // namespace lucid::math