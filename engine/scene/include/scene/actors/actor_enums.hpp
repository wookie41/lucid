﻿#pragma once

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
}