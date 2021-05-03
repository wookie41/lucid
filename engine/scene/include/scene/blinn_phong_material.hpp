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
        static CBlinnPhongMaterial* CreateMaterial(const FBlinnPhongMaterialDescription& Description, const FDString& InResourcePath);

        CBlinnPhongMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        void SaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;

        u32 Shininess;
        glm::vec3 DiffuseColor;
        glm::vec3 SpecularColor;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif
        
    };
    
    /////////////////////////////////////
    //      BlinnPhongMapsMaterial     //
    /////////////////////////////////////

    class CBlinnPhongMapsMaterial : public CMaterial
    {
      public:

        static CBlinnPhongMapsMaterial* CreateMaterial(const FBlinnPhongMapsMaterialDescription& Description, const FDString& InResourcePath);        
        
        CBlinnPhongMapsMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        void SaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
        
        u32 Shininess;
        resources::CTextureResource* DiffuseMap = nullptr;
        resources::CTextureResource* SpecularMap = nullptr;
        resources::CTextureResource* NormalMap = nullptr;
        resources::CTextureResource* DisplacementMap = nullptr;

        glm::vec3 SpecularColor; //Fallback when specular map is not used

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif

    };
} // namespace lucid::scene
