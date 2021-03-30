#pragma once

#include "scene/material.hpp"
#include "common/types.hpp"

namespace lucid::gpu
{
    class CShader;
};

namespace lucid::scene
{
    class FlatMaterial : public CMaterial
    {
      public:
        explicit FlatMaterial(gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        FColor Color;
    };
}