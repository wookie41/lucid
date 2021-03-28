#pragma once

#include <glm/vec3.hpp>


#include "common/strings.hpp"
#include "scene/material.hpp"

namespace lucid::resources
{
    class CMeshResource;
}

namespace lucid::scene
{
    struct FRenderable;

    /////////////////////////////////////
    //        BlinnPhongMaterial       //
    /////////////////////////////////////
    
    class CBlinnPhongMaterial : public CMaterial
    {
      public:
        explicit CBlinnPhongMaterial(gpu::CShader* CustomShader = nullptr);

        virtual void SetupShader(gpu::CShader* Shader) override;

        u32 Shininess;
        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;
    };

    /////////////////////////////////////
    //      BlinnPhongMapsMaterial     //
    /////////////////////////////////////

    class CBlinnPhongMapsMaterial : public CMaterial
    {
      public:
        explicit CBlinnPhongMapsMaterial(gpu::CShader* CustomShader = nullptr);

        virtual void SetupShader(gpu::CShader* Shader) override;

        u32 Shininess;
        gpu::CTexture* DiffuseMap = nullptr;
        gpu::CTexture* SpecularMap = nullptr;
        gpu::CTexture* NormalMap = nullptr;
        gpu::CTexture* DisplacementMap = nullptr;

        glm::vec3 SpecularColor; //Fallback when specular map is not used
    };

    FRenderable* CreateBlinnPhongRenderable(const FANSIString& InMeshName, resources::CMeshResource* InMesh, gpu::CShader* InShader);
} // namespace lucid::scene
