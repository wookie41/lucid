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

      private:

        inline void ReteriveShaderUniformsIds(gpu::Shader* Shader);

        // Material properties ids
        uint32_t shininessUniformId;
        uint32_t diffuseColorUniformId;
        uint32_t specularColorUniformId;
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
        gpu::Texture* DiffuseMap;
        gpu::Texture* SpecularMap;
        gpu::Texture* NormalMap;

      private:

        inline void ReteriveShaderUniformsIds(gpu::Shader* Shader);

        // Material properties ids
        uint32_t shininessUniformId;
        uint32_t diffuseMapUniformId;
        uint32_t specularMapUniformId;
        uint32_t normalMapUniformId;
    };

} // namespace lucid::scene
