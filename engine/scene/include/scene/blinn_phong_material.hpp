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

        virtual u32  SetupShader(char* InMaterialDataPtr) override;
        virtual void SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms) override;

        virtual CMaterial*    GetCopy() const override;
        virtual EMaterialType GetType() const override { return EMaterialType::BLINN_PHONG; }
        u16                   GetShaderDataSize() const override;

        u32       Shininess = 32;
        glm::vec3 DiffuseColor{ 1, 1, 1 };
        glm::vec3 SpecularColor{ 1, 1, 1 };

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif

      protected:
        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };

    /////////////////////////////////////
    //      BlinnPhongMapsMaterial     //
    /////////////////////////////////////

    class CBlinnPhongMapsMaterial : public CMaterial
    {
      public:
        CBlinnPhongMapsMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        static CBlinnPhongMapsMaterial* CreateMaterial(const FBlinnPhongMapsMaterialDescription& Description, const FDString& InResourcePath);

        virtual u32  SetupShader(char* InMaterialDataPtr) override;
        virtual void SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms) override;

        virtual CMaterial*    GetCopy() const override;
        virtual EMaterialType GetType() const override { return EMaterialType::BLINN_PHONG_MAPS; }
        u16                   GetShaderDataSize() const override;

        virtual void LoadResources();
        virtual void UnloadResources();

        u32 Shininess = 32;

        resources::CTextureResource* DiffuseMap               = nullptr;
        u64                          DiffuseMapBindlessHandle = 0;

        resources::CTextureResource* SpecularMap               = nullptr;
        u64                          SpecularMapBindlessHandle = 0;

        resources::CTextureResource* NormalMap               = nullptr;
        u64                          NormalMapBindlessHandle = 0;

        resources::CTextureResource* DisplacementMap               = nullptr;
        u64                          DisplacementMapBindlessHandle = 0;

        glm::vec3 SpecularColor{ 1, 1, 1 }; // Fallback when specular map is not used

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif
      protected:
        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };
} // namespace lucid::scene
