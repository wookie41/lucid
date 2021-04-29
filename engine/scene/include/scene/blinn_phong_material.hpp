#pragma once

#include <glm/vec3.hpp>

#include "common/strings.hpp"

#include "scene/actors/actor.hpp"
#include "scene/material.hpp"
#include "schemas/types.hpp"

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
        CBlinnPhongMaterial(const FString& InName, gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        u32 Shininess;
        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;
    };

    CBlinnPhongMaterial* CreateBlinnPhongMaterial(const FBlinnPhongMaterialDescription& Description);

    
    /////////////////////////////////////
    //      BlinnPhongMapsMaterial     //
    /////////////////////////////////////

    class CBlinnPhongMapsMaterial : public CMaterial
    {
      public:

        CBlinnPhongMapsMaterial(const FString& InName, gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        u32 Shininess;
        gpu::CTexture* DiffuseMap = nullptr;
        gpu::CTexture* SpecularMap = nullptr;
        gpu::CTexture* NormalMap = nullptr;
        gpu::CTexture* DisplacementMap = nullptr;

        glm::vec3 SpecularColor; //Fallback when specular map is not used
    };

    CBlinnPhongMapsMaterial* CreateBlinnPhongMapsMaterial(const FBlinnPhongMapsMaterialDescription& Description);
} // namespace lucid::scene
