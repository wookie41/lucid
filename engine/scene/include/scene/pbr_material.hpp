#pragma once

#include "scene/material.hpp"
#include "schemas/types.hpp"

namespace lucid::scene
{
    class CPBRMaterial : public scene::CMaterial
    {
      public:
        CPBRMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        static CPBRMaterial* CreateMaterial(const FPBRMaterialDescription& Description, const FDString& InResourcePath);

        virtual void SetupShaderBuffer(char* InMaterialDataPtr) override;
        virtual void SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms) override;

        virtual CMaterial*    GetCopy() const override;
        virtual EMaterialType GetType() const override { return EMaterialType::PBR; }
        u16                   GetShaderDataSize() const override;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif

      protected:
        glm::vec3 Albedo{ 1 };
        float     Metallic  = 0;
        float     Roughness = 0;

        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };
} // namespace lucid::scene
