#pragma once

#include "scene/material.hpp"
#include "schemas/types.hpp"

namespace lucid::scene
{
    class CTexturedPBRMaterial : public scene::CMaterial
    {
      public:
        CTexturedPBRMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        static CTexturedPBRMaterial* CreateMaterial(const FTexturedPBRMaterialDescription& Description, const FDString& InResourcePath);

        virtual void SetupShaderBuffer(char* InMaterialDataPtr) override;
        virtual void SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms) override;

        virtual CMaterial*    GetCopy() const override;
        virtual EMaterialType GetType() const override { return EMaterialType::TEXTURED_PBR; }
        u16                   GetShaderDataSize() const override;

        void LoadResources() override;
        void UnloadResources() override;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif

      protected:
        u64 RoughnessMapBindlessHandle    = 0;
        u64 MetallicMapBindlessHandle     = 0;
        u64 AlbedoMapBindlessHandle       = 0;
        u64 AOMapBindlessHandle       = 0;
        u64 NormalMapBindlessHandle       = 0;
        u64 DisplacementMapBindlessHandle = 0;

        resources::CTextureResource* RoughnessMap    = nullptr;
        resources::CTextureResource* MetallicMap     = nullptr;
        resources::CTextureResource* AlbedoMap       = nullptr;
        resources::CTextureResource* AOMap       = nullptr;
        resources::CTextureResource* NormalMap       = nullptr;
        resources::CTextureResource* DisplacementMap = nullptr;

        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };
} // namespace lucid::scene
