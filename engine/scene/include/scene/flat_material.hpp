#pragma once

#include "scene/material.hpp"
#include "common/types.hpp"
#include "schemas/types.hpp"

namespace lucid::gpu
{
    class CShader;
};

namespace lucid::scene
{
    class FlatMaterial : public CMaterial
    {
      public:
        FlatMaterial(const FString& InName, gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        FColor Color;
    };

    FlatMaterial* CreateFlatMaterial(const FFlatMaterialDescription& Description);
}