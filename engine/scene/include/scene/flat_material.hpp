#pragma once

#include "scene/material.hpp"
#include "common/types.hpp"
#include "schemas/types.hpp"

namespace lucid::gpu
{
    class CShader;
};

namespace lucid::scene
{
    class CFlatMaterial : public CMaterial
    {
      public:
        CFlatMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        static CFlatMaterial* CreateMaterial(const FFlatMaterialDescription& Description, const FDString& InResourcePath);

        virtual void SetupShaderBuffers(char* InMaterialDataPtr, u64* InBindlessTexturesArrayPtr, u32& OutMaterialDataSize, u32& BindlessTexturesSize) override;
        virtual void SetupPrepassShaderBuffers(FForwardPrepassUniforms* InPrepassUniforms) override;

        virtual CMaterial*    GetCopy() const override;
        virtual EMaterialType GetType() const override { return EMaterialType::FLAT; }
        u16                   GetShaderDataSize() const override;

        FColor Color;

      protected:
        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif
    };
} // namespace lucid::scene