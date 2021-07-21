#pragma once

#include "devices/gpu/texture.hpp"
#include "scene/material.hpp"
#include "schemas/types.hpp"

namespace lucid::scene
{
    constexpr u8 MAX_TERRAIN_LAYERS = 8;

    struct FTerrainLayer
    {
        float                        MaxHeight = 0;
        glm::vec2                    UVTiling{ 1, 1 };
        resources::CTextureResource* Diffuse = nullptr;
        resources::CTextureResource* Normal  = nullptr;
    };

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

        inline void AddLayer(const FTerrainLayer& InTerrainLayer)
        {
            assert(NumLayers < MAX_TERRAIN_LAYERS);
            TerrainLayers[NumLayers++] = InTerrainLayer;
        }

        virtual void LoadResources() override;
        virtual void UnloadResources() override;

#if DEVELOPMENT
        int          EditedLayer = -1;
        virtual void UIDrawMaterialEditor() override;
#endif

      protected:
        u8            NumLayers = 0;
        FTerrainLayer TerrainLayers[MAX_TERRAIN_LAYERS];

        void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) override;
    };
} // namespace lucid::scene
