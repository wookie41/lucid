#pragma once

#include "scene/material.hpp"

namespace lucid::gpu
{
    class Texture;
}


namespace lucid::scene
{
    class BlinnPhongMapsMaterial : public Material
    {
      public:
        explicit BlinnPhongMapsMaterial(gpu::Shader* CustomShader = nullptr);

        virtual void SetupShader(gpu::Shader* Shader) override;

        // Material properties

        float Shininess;
        gpu::Texture* DiffuseMap = nullptr;
        gpu::Texture* SpecularMap = nullptr;

      private:
        inline void ReteriveShaderUniformsIds(gpu::Shader* Shader);

        // Shader which uniform ids we've cached during the last SetupShader call
        gpu::Shader* cachedShader = nullptr;

        // Material properties ids

        uint32_t shininessUniformId;
        uint32_t diffuseMapUniformId;
        uint32_t specularMapUniformId;
    };
} // namespace lucid::scene
