#pragma once

#include "glm/vec4.hpp"

namespace lucid
{


    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using real = float;
    
    using color = glm::vec4;

    const color RedColor{ 1, 0, 0, 1 };
    const color GreenColor{ 0, 1, 0, 1 };
    const color BlueColor{ 0, 0, 1, 1 };
    const color WhiteColor{ 1, 1, 1, 1 };
    const color BlackColor{ 0, 0, 0, 1 };

    enum class Type : u8
    {
        BYTE,
        INT_8,
        INT_16,
        INT_32,
        INT_64,

        BOOL,
        
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
        MAT4,

        SAMPLER,

        UNSUPPORTED
    };

} // namespace lucid