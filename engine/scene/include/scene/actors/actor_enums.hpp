#pragma once

#include "common/types.hpp"

namespace lucid::scene
{
    enum class EStaticMeshType : u8
    {
        STATIONARY,
        MOVABLE
    };

    enum class EActorType : u8
    {
        UNKNOWN,
        STATIC_MESH,
        SKYBOX,
        LIGHT,
        TERRAIN
    };

    enum class ELightType : u8
    {
        DIRECTIONAL = 1,
        POINT,
        SPOT
    };

    enum class ELightUnit : u8
    {
        LUMENS,
        WATTS
    };

    enum class ELightSourceType : u8
    {
        INCANDESCENT,
        LED,
        FLUORESCENT
    };
} // namespace lucid::scene