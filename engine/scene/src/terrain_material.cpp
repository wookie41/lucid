#include "scene/terrain_material.hpp"

#include <glad/glad.h>
#include <lucid_editor/imgui_lucid.h>

#include "scene/forward_renderer.hpp"

#include "schemas/binary.hpp"
#include "schemas/json.hpp"

#include "engine/engine.hpp"

#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    CTerrainMaterial::CTerrainMaterial(const UUID& InAssetId, const FDString& InName, const FDString& InAssetPath, gpu::CShader* InShader)
    : CMaterial(InAssetId, InName, InAssetPath, InShader)
    {
    }

    CTerrainMaterial* CTerrainMaterial::CreateMaterial(const FTerrainMaterialDescription& InMaterialDescription, const FDString& InAssetPath)
    {
        gpu::CShader* Shader   = GEngine.GetShadersManager().GetShaderByName(InMaterialDescription.ShaderName);
        auto*         Material = new CTerrainMaterial{ InMaterialDescription.Id, InMaterialDescription.Name, InAssetPath, Shader };

        Material->NumLayers = 0;
        for (const auto& LayerDescription : InMaterialDescription.TerrainLayers)
        {
            assert(Material->NumLayers <= MAX_TERRAIN_LAYERS);
            const u8 LayerNumber = Material->NumLayers++;

            FTerrainLayer& TerrainLayer = Material->TerrainLayers[LayerNumber];
            TerrainLayer.MaxHeight      = LayerDescription.MaxHeight;
            TerrainLayer.UVTiling.x     = LayerDescription.UVTiling[0];
            TerrainLayer.UVTiling.y     = LayerDescription.UVTiling[1];

            if (LayerDescription.DiffuseTextureID != sole::INVALID_UUID)
            {
                TerrainLayer.Diffuse = GEngine.GetTexturesHolder().Get(LayerDescription.DiffuseTextureID);
            }

            if (LayerDescription.NormalTextureID != sole::INVALID_UUID)
            {
                TerrainLayer.Normal = GEngine.GetTexturesHolder().Get(LayerDescription.NormalTextureID);
            }
        }

        return Material;
    }

#pragma pack(push, 1)

    struct FTerrainLayerData
    {
        u64       DiffuseMap;
        u64       NormalMap;
        glm::vec2 UVTiling;
        float     MaxHeight;
        u32       bHasDiffuseMap;
        u32       bHasNormalMap;
        u8        _padding[4];
    };

    struct FTerrainMaterialData
    {
        FTerrainLayerData Layers[MAX_TERRAIN_LAYERS];
        u32               NumLayers;
        u8                _padding[4];
    };

#pragma pack(pop)

    void CTerrainMaterial::SetupShaderBuffer(char* InMaterialDataPtr)
    {
        CMaterial::SetupShaderBuffer(InMaterialDataPtr);

        FTerrainMaterialData* MaterialData = (FTerrainMaterialData*)InMaterialDataPtr;
        for (u8 i = 0; i < NumLayers; ++i)
        {
            MaterialData->Layers[i].MaxHeight = TerrainLayers[i].MaxHeight;
            MaterialData->Layers[i].UVTiling  = TerrainLayers[i].UVTiling;

            if (TerrainLayers[i].Diffuse)
            {
                MaterialData->Layers[i].DiffuseMap     = TerrainLayers[i].Diffuse->TextureHandle->GetBindlessHandle();
                MaterialData->Layers[i].bHasDiffuseMap = true;
            }
            else
            {
                MaterialData->Layers[i].DiffuseMap     = 0;
                MaterialData->Layers[i].bHasDiffuseMap = false;
            }

            if (TerrainLayers[i].Normal)
            {
                MaterialData->Layers[i].NormalMap     = TerrainLayers[i].Normal->TextureHandle->GetBindlessHandle();
                MaterialData->Layers[i].bHasNormalMap = true;
            }
            else
            {
                MaterialData->Layers[i].NormalMap     = 0;
                MaterialData->Layers[i].bHasNormalMap = false;
            }
        }
        MaterialData->NumLayers = NumLayers;
    }

    void CTerrainMaterial::SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms)
    {
        InPrepassUniforms->bHasDisplacementMap = false;
        InPrepassUniforms->bHasNormalMap       = false;
        // @TODO handle prepass for terrain - they have multiple normal maps - each per level
    }

    CMaterial* CTerrainMaterial::GetCopy() const
    {
        auto* Copy      = new CTerrainMaterial{ AssetId, Name, AssetPath, Shader };
        Copy->NumLayers = NumLayers;

        // @NOTE Currently there is a 1:1 mapping between terrain asset and terrain material, relax this requirement if it's actually needed
        
        memcpy(Copy->TerrainLayers, TerrainLayers, sizeof(TerrainLayers));
        return Copy;
    }

    u16 CTerrainMaterial::GetShaderDataSize() const { return sizeof(FTerrainMaterialData); }

    void CTerrainMaterial::LoadResources()
    {
        CMaterial::LoadResources();

        for (int i = 0; i < NumLayers; ++i)
        {
            const FTerrainLayer& TerrainLayer = TerrainLayers[i];

            if (TerrainLayer.Diffuse)
            {
                TerrainLayer.Diffuse->Acquire(false, true);
                TerrainLayer.Diffuse->TextureHandle->GetBindlessHandle();
                TerrainLayer.Diffuse->TextureHandle->MakeBindlessResident();
            }

            if (TerrainLayer.Normal)
            {
                TerrainLayer.Normal->Acquire(false, true);
                TerrainLayer.Normal->TextureHandle->GetBindlessHandle();
                TerrainLayer.Normal->TextureHandle->MakeBindlessResident();
            }
        }
    }

    void CTerrainMaterial::UnloadResources()
    {
        for (int i = 0; i < NumLayers; ++i)
        {
            const FTerrainLayer& TerrainLayer = TerrainLayers[i];

            if (TerrainLayer.Diffuse)
            {
                TerrainLayer.Diffuse->Release();
            }

            if (TerrainLayer.Normal)
            {
                TerrainLayer.Normal->Release();
            }
        }
    }

    void CTerrainMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();

        // @TODO handle base asset mesh resource changed
        ImGui::Text("Layers");
        for (u8 i = 0; i < NumLayers; ++i)
        {
            static char LayerLabelBuff[64];
            sprintf(LayerLabelBuff, "Layer %d", i);
            if (ImGui::CollapsingHeader(LayerLabelBuff))
            {
                if (ImGui::InputFloat("Layer max height", &TerrainLayers[i].MaxHeight))
                {
                    bMaterialDataDirty = true;
                }

                if (ImGui::InputFloat2("Layer UV tiling", &TerrainLayers[i].UVTiling.x))
                {
                    bMaterialDataDirty = true;
                }

                resources::CTextureResource* OldDiffuseTexture = TerrainLayers[i].Diffuse;
                if (ImGuiTextureResourcePicker("Diffuse", &TerrainLayers[i].Diffuse))
                {
                    if (OldDiffuseTexture)
                    {
                        OldDiffuseTexture->Release();
                    }

                    bMaterialDataDirty = true;
                    if (TerrainLayers[i].Diffuse)
                    {
                        TerrainLayers[i].Diffuse->Acquire(false, true);
                        TerrainLayers[i].Diffuse->TextureHandle->GetBindlessHandle();
                        TerrainLayers[i].Diffuse->TextureHandle->MakeBindlessResident();
                    }
                }

                resources::CTextureResource* OldNormalTexture = TerrainLayers[i].Normal;
                if (ImGuiTextureResourcePicker("Normal", &TerrainLayers[i].Normal))
                {
                    if (OldNormalTexture)
                    {
                        OldNormalTexture->Release();
                    }

                    bMaterialDataDirty = true;
                    if (TerrainLayers[i].Normal)
                    {
                        TerrainLayers[i].Normal->Acquire(false, true);
                        TerrainLayers[i].Normal->TextureHandle->GetBindlessHandle();
                        TerrainLayers[i].Normal->TextureHandle->MakeBindlessResident();
                    }
                }
            }
        }

        if (ImGui::Button("Add layer"))
        {
            if (NumLayers < MAX_TERRAIN_LAYERS)
            {
                ++NumLayers;
                bMaterialDataDirty = true;
            }
        }

        if (ImGui::Button("Remove last layer"))
        {
            if (NumLayers > 0)
            {
                --NumLayers;
                bMaterialDataDirty = true;
            }
        }
    }

    void CTerrainMaterial::InternalSaveToResourceFile(const EFileFormat& InFileFormat)
    {
        FTerrainMaterialDescription TerrainMaterialDescription;
        TerrainMaterialDescription.Id         = AssetId;
        TerrainMaterialDescription.Name       = FDString{ *Name, Name.GetLength() };
        TerrainMaterialDescription.ShaderName = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };

        for (int i = 0; i < NumLayers; ++i)
        {
            FTerrainMaterialLayerDescription LayerDescription;
            LayerDescription.MaxHeight   = TerrainLayers[i].MaxHeight;
            LayerDescription.UVTiling[0] = TerrainLayers[i].UVTiling.x;
            LayerDescription.UVTiling[1] = TerrainLayers[i].UVTiling.y;

            if (TerrainLayers[i].Diffuse)
            {
                LayerDescription.DiffuseTextureID = TerrainLayers[i].Diffuse->GetID();
            }

            if (TerrainLayers[i].Normal)
            {
                LayerDescription.NormalTextureID = TerrainLayers[i].Normal->GetID();
            }

            TerrainMaterialDescription.TerrainLayers.push_back(LayerDescription);
        }

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(TerrainMaterialDescription, *AssetPath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(TerrainMaterialDescription, *AssetPath);
            break;
        }
    }

} // namespace lucid::scene