#pragma once

#include "devices/gpu/texture.hpp"
#include "scene/material.hpp"
#include "schemas/types.hpp"

namespace lucid::scene
{
    class CTerrainMaterial : public CMaterial
    {
      public:
        CTerrainMaterial(const UUID& InAssetId, const FDString& InName, const FDString& InAssetPath, gpu::CShader* InShader);

        static CTerrainMaterial* CreateMaterial(const FTerrainMaterialDescription& InMaterialDescription, const FDString& InAssetPath);

        virtual void SetupShaderBuffer(char* InMaterialDataPtr) override;
        virtual void SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms) override;

        virtual CMaterial*    GetCopy() const override;
        virtual EMaterialType GetType() const override { return EMaterialType::TERRAIN; }
        u16                   GetShaderDataSize() const override;

        inline void           SetDeltaHeightMap(gpu::CTexture* InDeltaHeightMap) { DeltaHeightMap = InDeltaHeightMap; }

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() override;
#endif

      protected:
        gpu::CTexture* DeltaHeightMap = nullptr;

        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };
} // namespace lucid::scene
