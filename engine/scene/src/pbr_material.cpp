#include "scene/pbr_material.hpp"

#include "engine/engine.hpp"
#include "scene/forward_renderer.hpp"
#include "devices/gpu/shader.hpp"
#include "schemas/binary.hpp"
#include "schemas/json.hpp"

namespace lucid::scene
{
    CPBRMaterial::CPBRMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    CPBRMaterial* CPBRMaterial::CreateMaterial(const FPBRMaterialDescription& Description, const FDString& InResourcePath)
    {
        gpu::CShader* Shader   = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto*         Material = new CPBRMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->Albedo    = Description.Albedo;
        Material->Metallic  = Description.Metallic;
        Material->Roughness = Description.Roughness;

        return Material;
    }

#pragma pack(push, 1)
    struct FPBRMaterialData
    {
        glm::vec3 Albedo;
        float     Metallic;
        float     Roughness;
        u8        _padding[12];
    };
#pragma pack(pop)

    void CPBRMaterial::SetupShaderBuffer(char* InMaterialDataPtr)
    {
        CMaterial::SetupShaderBuffer(InMaterialDataPtr);

        FPBRMaterialData* MaterialData = (FPBRMaterialData*)InMaterialDataPtr;
        MaterialData->Albedo           = Albedo;
        MaterialData->Roughness        = Roughness;
        MaterialData->Metallic         = Metallic;
    }

    void CPBRMaterial::SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms)
    {
        InPrepassUniforms->bHasNormalMap       = false;
        InPrepassUniforms->bHasDisplacementMap = false;
    }

    CMaterial* CPBRMaterial::GetCopy() const
    {
        auto* Copy      = new CPBRMaterial{ AssetId, Name, AssetPath, Shader };
        Copy->Albedo    = Albedo;
        Copy->Roughness = Roughness;
        Copy->Metallic  = Metallic;
        return Copy;
    }

    lucid::u16 CPBRMaterial::GetShaderDataSize() const { return sizeof(FPBRMaterialData); }

    void CPBRMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();

        bMaterialDataDirty |= ImGui::ColorEdit3("Albedo", &Albedo.r);
        bMaterialDataDirty |= ImGui::DragFloat("Metallic", &Metallic, 0.01, 0, 1);
        bMaterialDataDirty |= ImGui::DragFloat("Roughness", &Roughness, 0.01, 0, 1);
    }

    void CPBRMaterial::InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat)
    {
        FPBRMaterialDescription PBRMaterialDescription;
        PBRMaterialDescription.Id         = AssetId;
        PBRMaterialDescription.Name       = FDString{ *Name, Name.GetLength() };
        PBRMaterialDescription.ShaderName = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };
        PBRMaterialDescription.Albedo     = Albedo;
        PBRMaterialDescription.Roughness  = Roughness;
        PBRMaterialDescription.Metallic   = Metallic;

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(PBRMaterialDescription, *AssetPath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(PBRMaterialDescription, *AssetPath);
            break;
        }
    }

} // namespace lucid::scene