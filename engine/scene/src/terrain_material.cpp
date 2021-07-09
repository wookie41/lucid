#include "scene/terrain_material.hpp"
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
        return Material;
    }

#pragma pack(push, 1)
    struct FTerrainMaterialData
    {
        u64  DeltaHeightMapBindlessHandle;
        u32  bHasDeltaHeightMap;
        char padding[4];
    };
#pragma pack(pop)

    void CTerrainMaterial::SetupShaderBuffer(char* InMaterialDataPtr)
    {
        CMaterial::SetupShaderBuffer(InMaterialDataPtr);

        FTerrainMaterialData* MaterialData         = (FTerrainMaterialData*)InMaterialDataPtr;
        MaterialData->bHasDeltaHeightMap           = DeltaHeightMap != nullptr;
        MaterialData->DeltaHeightMapBindlessHandle = DeltaHeightMap ? DeltaHeightMap->GetBindlessHandle() : 0;
    }

    void CTerrainMaterial::SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms)
    {
        InPrepassUniforms->bHasDisplacementMap = false;
        InPrepassUniforms->bHasNormalMap       = false;
    }

    CMaterial* CTerrainMaterial::GetCopy() const
    {
        auto* Copy = new CTerrainMaterial{ AssetId, Name, AssetPath, Shader };
        return Copy;
    }

    u16 CTerrainMaterial::GetShaderDataSize() const { return sizeof(FTerrainMaterialData); }

    void CTerrainMaterial::UIDrawMaterialEditor() { CMaterial::UIDrawMaterialEditor(); }

    void CTerrainMaterial::InternalSaveToResourceFile(const EFileFormat& InFileFormat)
    {
        FTerrainMaterialDescription TerrainMaterialDescription;
        TerrainMaterialDescription.Id         = AssetId;
        TerrainMaterialDescription.Name       = FDString{ *Name, Name.GetLength() };
        TerrainMaterialDescription.ShaderName = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };

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