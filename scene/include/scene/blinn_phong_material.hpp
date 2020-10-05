#pragma once

#include "scene/material.hpp"

namespace lucid::scene
{
    class BlinnPhongMaterial : public Material
    {
      public:
        explicit BlinnPhongMaterial(gpu::Shader* CustomShader = nullptr);

        virtual void SetupShader(gpu::Shader* Shader) override;

        // Material properties

        float Shininess;
        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;

      private:

        inline void ReteriveShaderUniformsIds(gpu::Shader* Shader);

        // Shader which uniform ids we've cached during the last SetupShader call
        gpu::Shader* cachedShader = nullptr;

        // Material properties ids

        uint32_t shininessUniformId;
        uint32_t diffuseColorUniformId;
        uint32_t specularColorUniformId;
    };
} // namespace lucid::scene
