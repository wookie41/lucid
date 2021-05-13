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

        CBlinnPhongMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        static CBlinnPhongMaterial* CreateMaterial(const FBlinnPhongMaterialDescription& Description, const FDString& InResourcePath);

        virtual void                SetupShader(gpu::CShader* Shader) override;
        virtual CMaterial*          GetCopy() const override;
        virtual EMaterialType       GetType() const override { return EMaterialType::BLINN_PHONG; }

        u32 Shininess = 32;
        glm::vec3 DiffuseColor  { 1, 1, 1 };
        glm::vec3 SpecularColor { 1, 1, 1 };

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif

    protected:
        void                        InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };
    
    /////////////////////////////////////
    //      BlinnPhongMapsMaterial     //
    /////////////////////////////////////

    class CBlinnPhongMapsMaterial : public CMaterial
    {
      public:

        CBlinnPhongMapsMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        static CBlinnPhongMapsMaterial* CreateMaterial(const FBlinnPhongMapsMaterialDescription& Description, const FDString& InResourcePath);        

        virtual void                    SetupShader(gpu::CShader* Shader) override;
        virtual CMaterial*              GetCopy() const override;
        virtual EMaterialType           GetType() const override { return EMaterialType::BLINN_PHONG_MAPS; }

        u32 Shininess = 32;
        resources::CTextureResource* DiffuseMap = nullptr;
        resources::CTextureResource* SpecularMap = nullptr;
        resources::CTextureResource* NormalMap = nullptr;
        resources::CTextureResource* DisplacementMap = nullptr;

        glm::vec3 SpecularColor { 1, 1, 1 }; //Fallback when specular map is not used

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif
    protected:
        void                            InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
        
    };
} // namespace lucid::scene
