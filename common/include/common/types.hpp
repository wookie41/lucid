#pragma once

#include "common/math.hpp"

namespace lucid
{
    enum Type
    {
        BYTE,
        INT_8,
        INT_16,
        INT_32,
        INT_64,

        UINT_8,
        UINT_16,
        UINT_32,
        UINT_64,

        FLOAT,
        DOUBLE,

        VEC2,
        VEC3,
        VEC4,

        IVEC2,
        IVEC3,
        IVEC4,

        MAT3,
        MAT4
    };

    using color = math::vec4;

    const color RedColor{ 1, 0, 0, 1 };
    const color GreenColor{ 0, 1, 0, 1 };
    const color BlueColor{ 0, 0, 1, 1 };
    const color WhiteColor{ 1, 1, 1, 1 };
    const color BlackColor{ 0, 0, 0, 1 };

} // namespace lucid