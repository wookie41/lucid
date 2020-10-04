#pragma once

#include "scene/material.hpp"

namespace lucid::scene
{
    class BlinnPhongMaterial : public Material
    {
      public:
        explicit BlinnPhongMaterial(gpu::Shader* CustomShader = nullptr);

        virtual void SetupShader(gpu::Shader* Shader = nullptr) override;

        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;
    };
} // namespace lucid::scene
