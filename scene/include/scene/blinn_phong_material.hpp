#pragma once

#include <glm/vec3.hpp>

#include "scene/material.hpp"

namespace lucid::resources
{
    class CMeshResource;
}

namespace lucid::scene
{
    struct FRenderable;

    // The most basic material compatible with the BlinnPhongRenderer
    // It'll setup the diffuse and specular colors which will be used over the entire surface
    class CBlinnPhongMaterial : public CMaterial
    {
      public:
        explicit CBlinnPhongMaterial(gpu::CShader* CustomShader = nullptr);

        virtual void SetupShader(gpu::CShader* Shader) override;

        // Material properties
        u32 Shininess;
        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;
    };

    /* ------------------------------------------------------------ */

    // Material compatible with the BlinnPhongRenderer
    // It'll setup the diffuse, specular and normal maps that wil be used to render the surface
    class CBlinnPhongMapsMaterial : public CMaterial
    {
      public:
        explicit CBlinnPhongMapsMaterial(gpu::CShader* CustomShader = nullptr);

        virtual void SetupShader(gpu::CShader* Shader) override;

        // Material properties
        u32 Shininess;
        gpu::CTexture* DiffuseMap = nullptr;
        gpu::CTexture* SpecularMap = nullptr;
        gpu::CTexture* NormalMap = nullptr;
        gpu::CTexture* DisplacementMap = nullptr;

        glm::vec3 SpecularColor; //Fallback when specular map is not used
    };

    FRenderable* CreateBlinnPhongRenderable(const FANSIString& InMeshName, resources::CMeshResource* InMesh, gpu::CShader* InShader);
} // namespace lucid::scene
