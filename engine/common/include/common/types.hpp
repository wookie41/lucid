#pragma once

#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>
#include <array>

#include "glm/vec4.hpp"
#include "sole/sole.hpp"

namespace lucid
{
    constexpr float FLT_HALF_MIN = FLT_MIN / 2.f;
    constexpr float FLT_HALF_MAX = FLT_MAX / 2.f;

    using UUID = sole::uuid;

    using u8   = uint8_t;
    using u16  = uint16_t;
    using u32  = uint32_t;
    using u64  = uint64_t;
    using i8   = int8_t;
    using i16  = int16_t;
    using i32  = int32_t;
    using i64  = int64_t;
    using real = float;

    using FColor = glm::vec4;

    const FColor    RedColor{ 1, 0, 0, 1 };
    const FColor    GreenColor{ 0, 1, 0, 1 };
    const FColor    BlueColor{ 0, 0, 1, 1 };
    const FColor    WhiteColor{ 1, 1, 1, 1 };
    const FColor    BlackColor{ 0, 0, 0, 1 };
    const glm::vec3 One{ 1, 1, 1 };

    enum class EType : u8
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

    inline glm::vec3 Float3ToVec(const std::array<float, 3>& Values) { return { Values[0], Values[1], Values[2] }; }

    inline glm::vec4 Float4ToVec(const std::array<float, 4>& Values) { return { Values[0], Values[1], Values[2], Values[3] }; }

    inline glm::quat Float4ToQuat(const std::array<float, 4>& Values) { return { Values[3], Values[0], Values[1], Values[2] }; }

    inline std::array<float, 3> VecToFloat3(const glm::vec3& Vec) { return { Vec.x, Vec.y, Vec.z }; }

    inline std::array<float, 4> VecToFloat4(const glm::vec4& Vec) { return { Vec.x, Vec.y, Vec.z, Vec.w }; }

    inline std::array<float, 4> QuatToFloat4(const glm::quat& Quat) { return { Quat.x, Quat.y, Quat.z, Quat.w }; }

    /**
     * Helper struct used when there are objects that reference a base resource and change just some of it's properties
     * E.x. CStaticMesh that uses a differnt mesh, but the material
     */
    template <typename T>
    struct InstancedVariable
    {
        InstancedVariable() = default;

        T    Value    = T{};
        bool bChanged = false;

        bool operator==(const InstancedVariable<T>& InRhs) const { return Value == InRhs.Value; }
        bool operator!=(const InstancedVariable<T>& InRhs) const { return Value != InRhs.Value; }
    };
} // namespace lucid