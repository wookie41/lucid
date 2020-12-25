#pragma once

#include "scene/material.hpp"

namespace lucid::scene
{
    // The most basic material compatible with the BlinnPhongRenderer
    // It'll setup the diffuse and specular colors which will be used over the entire surface
    class BlinnPhongMaterial : public Material
    {
      public:
        explicit BlinnPhongMaterial(gpu::Shader* CustomShader = nullptr);

        virtual void SetupShader(gpu::Shader* Shader) override;

        // Material properties
        uint32_t Shininess;
        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;
    };

    /* ------------------------------------------------------------ */

    // Material compatible with the BlinnPhongRenderer
    // It'll setup the diffuse, specular and normal maps that wil be used to render the surface
    class BlinnPhongMapsMaterial : public Material
    {
      public:
        explicit BlinnPhongMapsMaterial(gpu::Shader* CustomShader = nullptr);

        virtual void SetupShader(gpu::Shader* Shader) override;

        // Material properties
        uint32_t Shininess;
        gpu::Texture* DiffuseMap = nullptr;
        gpu::Texture* SpecularMap = nullptr;
        gpu::Texture* NormalMap = nullptr;
        gpu::Texture* DisplacementMap = nullptr;

        glm::vec3 SpecularColor; //Fallback when specular map is not used
    };
} // namespace lucid::scene
