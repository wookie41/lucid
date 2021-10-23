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
        AlbedoMap = NormalMap = RoughnessMap = MetallicMap = DisplacementMap = AOMap = nullptr;
    }

    CTexturedPBRMaterial* CTexturedPBRMaterial::CreateMaterial(const FTexturedPBRMaterialDescription& Description, const FDString& InResourcePath)
    {
        gpu::CShader* Shader   = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto*         Material = new CTexturedPBRMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->RoughnessMap =
          Description.RoughnessTextureID == sole::INVALID_UUID ? nullptr : GEngine.GetTexturesHolder().Get(Description.RoughnessTextureID);

        Material->MetallicMap = Description.MetallicTextureID == sole::INVALID_UUID ? nullptr : GEngine.GetTexturesHolder().Get(Description.MetallicTextureID);
        Material->AlbedoMap   = Description.AlbedoTextureID == sole::INVALID_UUID ? nullptr : GEngine.GetTexturesHolder().Get(Description.AlbedoTextureID);
        Material->AOMap       = Description.AOTextureID == sole::INVALID_UUID ? nullptr : GEngine.GetTexturesHolder().Get(Description.AOTextureID);
        Material->NormalMap   = Description.NormalTextureID == sole::INVALID_UUID ? nullptr : GEngine.GetTexturesHolder().Get(Description.NormalTextureID);

        Material->DisplacementMap =
          Description.DisplacementTextureID == sole::INVALID_UUID ? nullptr : GEngine.GetTexturesHolder().Get(Description.DisplacementTextureID);

        Material->Albedo    = Description.Albedo;
        Material->Roughness = Description.Roughness;
        Material->Metallic  = Description.Metallic;

        return Material;
    }

#pragma pack(push, 1)

    enum PBRMaterialFlags
    {
        HAS_ALBEDO       = 1,
        HAS_NORMAL       = 2,
        HAS_METALLIC     = 4,
        HAS_ROUGHNESS    = 8,
        HAS_AO           = 16,
        HAS_DISPLACEMENT = 32
    };

    struct FTexturedPBRMaterialData
    {
        u64 RoughnessMapBindlessHandle;
        u64 MetallicMapBindlessHandle;
        u64 AlbedoMapBindlessHandle;
        u64 AOMapBindlessHandle;
        u64 NormalMapBindlessHandle;
        u64 DisplacementMapBindlessHandle;

        glm::vec3 Albedo    = { 1, 0, 0 };
        float     Roughness = 0;
        float     Metallic  = 0;

        u32 Flags;
        u8  _Padding[8];
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

        MaterialData->Flags = 0;

        if (AlbedoMap)
        {
            MaterialData->Flags |= PBRMaterialFlags::HAS_ALBEDO;
        }
        else
        {
            MaterialData->Albedo = Albedo;
        }

        if (NormalMap)
        {
            MaterialData->Flags |= PBRMaterialFlags::HAS_NORMAL;
        }

        if (RoughnessMap)
        {
            MaterialData->Flags |= PBRMaterialFlags::HAS_ROUGHNESS;
        }
        else
        {
            MaterialData->Roughness = Roughness;
        }

        if (MetallicMap)
        {
            MaterialData->Flags |= PBRMaterialFlags::HAS_METALLIC;
        }
        else
        {
            MaterialData->Metallic = Metallic;
        }

        if (AOMap)
        {
            MaterialData->Flags |= PBRMaterialFlags::HAS_AO;
        }

        if (DisplacementMap)
        {
            MaterialData->Flags |= PBRMaterialFlags::HAS_DISPLACEMENT;
        }
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
        Copy->Albedo                        = Albedo;
        Copy->Roughness                     = Roughness;
        Copy->Metallic                      = Metallic;
        return Copy;
    }

    lucid::u16 CTexturedPBRMaterial::GetShaderDataSize() const { return sizeof(FTexturedPBRMaterialData); }

    void CTexturedPBRMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();

        bMaterialDataDirty |= ImGui::ColorEdit3("Albedo", &Albedo.x);
        bMaterialDataDirty |= ImGui::DragFloat("Roughness", &Roughness, 0.01, 0, 1);
        bMaterialDataDirty |= ImGui::DragFloat("Metallic", &Metallic, 0.01, 0, 1);

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

        MaterialDescription.Albedo    = Albedo;
        MaterialDescription.Metallic  = Metallic;
        MaterialDescription.Roughness = Roughness;

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