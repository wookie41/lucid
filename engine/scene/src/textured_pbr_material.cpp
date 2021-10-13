#include "scene/textured_pbr_material.hpp"

#include "engine/engine.hpp"
#include "scene/forward_renderer.hpp"
#include "devices/gpu/shader.hpp"
#include "schemas/binary.hpp"
#include "schemas/json.hpp"

#if DEVELOPMENT
#include "imgui.h"
#include "lucid_editor/imgui_lucid.h"
#endif

namespace lucid::scene
{
    CTexturedPBRMaterial::CTexturedPBRMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    CTexturedPBRMaterial* CTexturedPBRMaterial::CreateMaterial(const FTexturedPBRMaterialDescription& Description, const FDString& InResourcePath)
    {
        gpu::CShader* Shader   = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto*         Material = new CTexturedPBRMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->RoughnessMap = GEngine.GetTexturesHolder().Get(Description.RoughnessTextureID);
        Material->MetallicMap  = GEngine.GetTexturesHolder().Get(Description.MetallicTextureID);
        Material->AlbedoMap    = GEngine.GetTexturesHolder().Get(Description.AlbedoTextureID);
        Material->AOMap        = GEngine.GetTexturesHolder().Get(Description.AOTextureID);

        if (Description.NormalTextureID != sole::INVALID_UUID)
        {
            Material->NormalMap = GEngine.GetTexturesHolder().Get(Description.NormalTextureID);
        }

        if (Description.DisplacementTextureID != sole::INVALID_UUID)
        {
            Material->DisplacementMap = GEngine.GetTexturesHolder().Get(Description.DisplacementTextureID);
        }

        return Material;
    }

#pragma pack(push, 1)
    struct FTexturedPBRMaterialData
    {

        u64 RoughnessMapBindlessHandle;
        u64 MetallicMapBindlessHandle;
        u64 AlbedoMapBindlessHandle;
        u64 AOMapBindlessHandle;
        u64 NormalMapBindlessHandle;
        u64 DisplacementMapBindlessHandle;

        u32 bHasNormalMap;
        u32 bHasDisplacementMap;
    };
#pragma pack(pop)

    void CTexturedPBRMaterial::SetupShaderBuffer(char* InMaterialDataPtr)
    {
        CMaterial::SetupShaderBuffer(InMaterialDataPtr);

        FTexturedPBRMaterialData* MaterialData      = (FTexturedPBRMaterialData*)InMaterialDataPtr;
        MaterialData->RoughnessMapBindlessHandle    = RoughnessMapBindlessHandle;
        MaterialData->MetallicMapBindlessHandle     = MetallicMapBindlessHandle;
        MaterialData->AlbedoMapBindlessHandle       = AlbedoMapBindlessHandle;
        MaterialData->AOMapBindlessHandle           = AOMapBindlessHandle;
        MaterialData->NormalMapBindlessHandle       = NormalMapBindlessHandle;
        MaterialData->DisplacementMapBindlessHandle = DisplacementMapBindlessHandle;

        MaterialData->bHasNormalMap = NormalMap != nullptr;
        MaterialData->bHasDisplacementMap = DisplacementMap != nullptr;
    }

    void CTexturedPBRMaterial::SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms)
    {
        InPrepassUniforms->bHasNormalMap       = NormalMap != nullptr;
        InPrepassUniforms->bHasDisplacementMap = DisplacementMap != nullptr;

        InPrepassUniforms->NormalMapBindlessHandle       = NormalMapBindlessHandle;
        InPrepassUniforms->DisplacementMapBindlessHandle = DisplacementMapBindlessHandle;
    }

    CMaterial* CTexturedPBRMaterial::GetCopy() const
    {
        auto* Copy                          = new CTexturedPBRMaterial{ AssetId, Name, AssetPath, Shader };
        Copy->RoughnessMap                  = RoughnessMap;
        Copy->MetallicMap                   = MetallicMap;
        Copy->AlbedoMap                     = AlbedoMap;
        Copy->AOMap                         = AOMap;
        Copy->NormalMap                     = NormalMap;
        Copy->DisplacementMap               = DisplacementMap;
        Copy->RoughnessMapBindlessHandle    = RoughnessMapBindlessHandle;
        Copy->MetallicMapBindlessHandle     = MetallicMapBindlessHandle;
        Copy->AlbedoMapBindlessHandle       = AlbedoMapBindlessHandle;
        Copy->AOMapBindlessHandle           = AOMapBindlessHandle;
        Copy->NormalMapBindlessHandle       = NormalMapBindlessHandle;
        Copy->DisplacementMapBindlessHandle = DisplacementMapBindlessHandle;
        return Copy;
    }

    lucid::u16 CTexturedPBRMaterial::GetShaderDataSize() const { return sizeof(FTexturedPBRMaterialData); }

    void CTexturedPBRMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();

        ImGui::Text("Roughness texture:");
        resources::CTextureResource* OldRoughnessMap = RoughnessMap;
        if (ImGuiTextureResourcePicker("pbr_roughness", &RoughnessMap))
        {
            if (OldRoughnessMap)
            {
                OldRoughnessMap->Release();
            }
            if (RoughnessMap)
            {
                RoughnessMap->Acquire(false, true);
                RoughnessMapBindlessHandle = RoughnessMap->TextureHandle->GetBindlessHandle();
                RoughnessMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("Metallic texture:");
        resources::CTextureResource* OldMetallicMap = MetallicMap;
        if (ImGuiTextureResourcePicker("pbr_Metallic", &MetallicMap))
        {
            if (OldMetallicMap)
            {
                OldMetallicMap->Release();
            }
            if (MetallicMap)
            {
                MetallicMap->Acquire(false, true);
                MetallicMapBindlessHandle = MetallicMap->TextureHandle->GetBindlessHandle();
                MetallicMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("Albedo texture:");
        resources::CTextureResource* OldAlbedoMap = AlbedoMap;
        if (ImGuiTextureResourcePicker("pbr_Albedo", &AlbedoMap))
        {
            if (OldAlbedoMap)
            {
                OldAlbedoMap->Release();
            }
            if (AlbedoMap)
            {
                AlbedoMap->Acquire(false, true);
                AlbedoMapBindlessHandle = AlbedoMap->TextureHandle->GetBindlessHandle();
                AlbedoMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("AO texture:");
        resources::CTextureResource* OldAOMap = AOMap;
        if (ImGuiTextureResourcePicker("pbr_AO", &AOMap))
        {
            if (OldAOMap)
            {
                OldAOMap->Release();
            }
            if (AOMap)
            {
                AOMap->Acquire(false, true);
                AOMapBindlessHandle = AOMap->TextureHandle->GetBindlessHandle();
                AOMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("Normal texture:");
        resources::CTextureResource* OldNormalMap = NormalMap;
        if (ImGuiTextureResourcePicker("pbr_Normal", &NormalMap))
        {
            if (OldNormalMap)
            {
                OldNormalMap->Release();
            }
            if (NormalMap)
            {
                NormalMap->Acquire(false, true);
                NormalMapBindlessHandle = NormalMap->TextureHandle->GetBindlessHandle();
                NormalMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("Displacement texture:");
        resources::CTextureResource* OldDisplacementMap = DisplacementMap;
        if (ImGuiTextureResourcePicker("pbr_Displacement", &DisplacementMap))
        {
            if (OldDisplacementMap)
            {
                OldDisplacementMap->Release();
            }
            if (DisplacementMap)
            {
                DisplacementMap->Acquire(false, true);
                DisplacementMapBindlessHandle = DisplacementMap->TextureHandle->GetBindlessHandle();
                DisplacementMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }
    }

    void CTexturedPBRMaterial::InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat)
    {
        FTexturedPBRMaterialDescription MaterialDescription;
        MaterialDescription.Id         = AssetId;
        MaterialDescription.Name       = FDString{ *Name, Name.GetLength() };
        MaterialDescription.ShaderName = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };

        MaterialDescription.RoughnessTextureID    = RoughnessMap ? RoughnessMap->GetID() : sole::INVALID_UUID;
        MaterialDescription.MetallicTextureID     = MetallicMap ? MetallicMap->GetID() : sole::INVALID_UUID;
        MaterialDescription.AlbedoTextureID       = AlbedoMap ? AlbedoMap->GetID() : sole::INVALID_UUID;
        MaterialDescription.AOTextureID           = AOMap ? AOMap->GetID() : sole::INVALID_UUID;
        MaterialDescription.NormalTextureID       = NormalMap ? NormalMap->GetID() : sole::INVALID_UUID;
        MaterialDescription.DisplacementTextureID = DisplacementMap ? DisplacementMap->GetID() : sole::INVALID_UUID;

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(MaterialDescription, *AssetPath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(MaterialDescription, *AssetPath);
            break;
        }
    }

    void CTexturedPBRMaterial::LoadResources()
    {
        CMaterial::LoadResources();

        if (RoughnessMap)
        {
            RoughnessMap->Acquire(false, true);
            if (RoughnessMapBindlessHandle == 0)
            {
                RoughnessMapBindlessHandle = RoughnessMap->TextureHandle->GetBindlessHandle();
                RoughnessMap->TextureHandle->MakeBindlessResident();
            }
        }

        if (MetallicMap)
        {
            MetallicMap->Acquire(false, true);
            if (MetallicMapBindlessHandle == 0)
            {
                MetallicMapBindlessHandle = MetallicMap->TextureHandle->GetBindlessHandle();
                MetallicMap->TextureHandle->MakeBindlessResident();
            }
        }

        if (AlbedoMap)
        {
            AlbedoMap->Acquire(false, true);
            if (AlbedoMapBindlessHandle == 0)
            {
                AlbedoMapBindlessHandle = AlbedoMap->TextureHandle->GetBindlessHandle();
                AlbedoMap->TextureHandle->MakeBindlessResident();
            }
        }

        if (AOMap)
        {
            AOMap->Acquire(false, true);
            if (AOMapBindlessHandle == 0)
            {
                AOMapBindlessHandle = AOMap->TextureHandle->GetBindlessHandle();
                AOMap->TextureHandle->MakeBindlessResident();
            }
        }

        if (NormalMap)
        {
            NormalMap->Acquire(false, true);
            if (NormalMapBindlessHandle == 0)
            {
                NormalMapBindlessHandle = NormalMap->TextureHandle->GetBindlessHandle();
                NormalMap->TextureHandle->MakeBindlessResident();
            }
        }

        if (DisplacementMap)
        {
            DisplacementMap->Acquire(false, true);
            if (DisplacementMapBindlessHandle == 0)
            {
                DisplacementMapBindlessHandle = DisplacementMap->TextureHandle->GetBindlessHandle();
                DisplacementMap->TextureHandle->MakeBindlessResident();
            }
        }
    }

    void CTexturedPBRMaterial::UnloadResources()
    {
        if (RoughnessMap)
        {
            RoughnessMap->Release();
            RoughnessMapBindlessHandle = 0;
        }

        if (MetallicMap)
        {
            MetallicMap->Release();
            MetallicMapBindlessHandle = 0;
        }

        if (AlbedoMap)
        {
            AlbedoMap->Release();
            AlbedoMapBindlessHandle = 0;
        }

        if (AOMap)
        {
            AOMap->Release();
            AOMapBindlessHandle = 0;
        }

        if (NormalMap)
        {
            NormalMap->Release();
            NormalMapBindlessHandle = 0;
        }

        if (DisplacementMap)
        {
            DisplacementMap->Release();
            DisplacementMapBindlessHandle = 0;
        }
    }

} // namespace lucid::scene