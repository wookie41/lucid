#include "scene/blinn_phong_material.hpp"

#include "engine/engine.hpp"

#include "devices/gpu/shader.hpp"
#include "devices/gpu/fence.hpp"
#include "resources/texture_resource.hpp"

#include "scene/forward_renderer.hpp"

#include "schemas/binary.hpp"
#include "schemas/json.hpp"

#if DEVELOPMENT
#include "imgui.h"
#include "lucid_editor/imgui_lucid.h"
#endif

namespace lucid::scene
{

#pragma pack(push, 1)
#pragma pack(pop)

    CBlinnPhongMaterial::CBlinnPhongMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    // void CBlinnPhongMaterial::SetupShader(gpu::CShader* Shader)
    // {
    // Shader->SetInt(SHININESS, Shininess);
    // Shader->SetVector(DIFFUSE_COLOR, DiffuseColor);
    // Shader->SetVector(SPECULAR_COLOR, SpecularColor);
    // };

    CBlinnPhongMaterial* CBlinnPhongMaterial::CreateMaterial(const FBlinnPhongMaterialDescription& Description, const FDString& InResourcePath)
    {
        gpu::CShader* Shader   = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto*         Material = new CBlinnPhongMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->Shininess     = Description.Shininess;
        Material->SpecularColor = { Description.SpecularColor[0], Description.SpecularColor[1], Description.SpecularColor[2] };
        Material->DiffuseColor  = { Description.DiffuseColor[0], Description.DiffuseColor[1], Description.DiffuseColor[2] };

        return Material;
    }

#pragma pack(push, 1)
    struct FBlinnPhongMaterialData
    {
        glm::vec3 DiffuseColor;
        u32       Shininess;
        glm::vec3 SpecularColor;
        char      padding[4];
    };
#pragma pack(pop)

    void CBlinnPhongMaterial::SetupShaderBuffer(char* InMaterialDataPtr)
    {
        CMaterial::SetupShaderBuffer(InMaterialDataPtr);

        FBlinnPhongMaterialData* MaterialData = (FBlinnPhongMaterialData*)InMaterialDataPtr;
        MaterialData->DiffuseColor            = DiffuseColor;
        MaterialData->SpecularColor           = SpecularColor;
        MaterialData->Shininess               = Shininess;
    }

    void CBlinnPhongMaterial::SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms)
    {
        InPrepassUniforms->bHasNormalMap       = false;
        InPrepassUniforms->bHasDisplacementMap = false;
    }

    void CBlinnPhongMaterial::InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat)
    {
        FBlinnPhongMaterialDescription BlinnPhongMaterialDescription;
        BlinnPhongMaterialDescription.Id            = AssetId;
        BlinnPhongMaterialDescription.Name          = FDString{ *Name, Name.GetLength() };
        BlinnPhongMaterialDescription.ShaderName    = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };
        BlinnPhongMaterialDescription.Shininess     = Shininess;
        BlinnPhongMaterialDescription.DiffuseColor  = VecToFloat3(DiffuseColor);
        BlinnPhongMaterialDescription.SpecularColor = VecToFloat3(SpecularColor);

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(BlinnPhongMaterialDescription, *AssetPath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(BlinnPhongMaterialDescription, *AssetPath);
            break;
        }
    }

    void CBlinnPhongMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();
        if (ImGui::InputInt("Shininess", (int*)&Shininess))
        {
            bMaterialDataDirty = true;
        }

        if (ImGui::DragFloat3("Diffuse color", &DiffuseColor.r, 0.005, 0, 1))
        {
            bMaterialDataDirty = true;
        }
        if (ImGui::DragFloat3("Specular color", &SpecularColor.r, 0.005, 0, 1))
        {
            bMaterialDataDirty = true;
        }
    }

    CMaterial* CBlinnPhongMaterial::GetCopy() const
    {
        auto* Copy          = new CBlinnPhongMaterial{ AssetId, Name, AssetPath, Shader };
        Copy->Shininess     = Shininess;
        Copy->DiffuseColor  = DiffuseColor;
        Copy->SpecularColor = SpecularColor;
        return Copy;
    }

    u16 CBlinnPhongMaterial::GetShaderDataSize() const { return sizeof(FBlinnPhongMaterialData); }

    /* ---------------------------------------------------------------------------*/

    CBlinnPhongMapsMaterial::CBlinnPhongMapsMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    CBlinnPhongMapsMaterial* CBlinnPhongMapsMaterial::CreateMaterial(const FBlinnPhongMapsMaterialDescription& Description,
                                                                     const FDString&                           InResourcePath)
    {
        gpu::CShader* Shader   = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto*         Material = new CBlinnPhongMapsMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->Shininess  = Description.Shininess;
        Material->DiffuseMap = GEngine.GetTexturesHolder().Get(Description.DiffuseTextureID);
        if (Description.SpecularTextureID != sole::INVALID_UUID)
        {
            Material->SpecularMap = GEngine.GetTexturesHolder().Get(Description.SpecularTextureID);
        }
        else
        {
            Material->SpecularColor = Float3ToVec(Description.SpecularColor);
        }

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
    struct FBlinnPhongMapsMaterialData
    {
        glm::vec3 SpecularColor;
        u32       _padding;

        u64 DiffuseMapBindlessHandle;
        u64 SpecularMapBindlessHandle;
        u64 NormalMapBindlessHandle;
        u64 DisplacementMapBindlessHandle;

        u32 Shininess;
        u32 bHasSpecularMap;
        u32 bHasNormalMap;
        u32 bHasDisplacementMap;
    };
#pragma pack(pop)

    void CBlinnPhongMapsMaterial::SetupShaderBuffer(char* InMaterialDataPtr)
    {
        CMaterial::SetupShaderBuffer(InMaterialDataPtr);

        FBlinnPhongMapsMaterialData* MaterialData   = (FBlinnPhongMapsMaterialData*)InMaterialDataPtr;
        MaterialData->SpecularColor                 = SpecularColor;
        MaterialData->Shininess                     = Shininess;
        MaterialData->bHasSpecularMap               = SpecularMap != nullptr;
        MaterialData->bHasNormalMap                 = NormalMap != nullptr;
        MaterialData->bHasDisplacementMap           = DisplacementMap != nullptr;
        MaterialData->DiffuseMapBindlessHandle      = DiffuseMapBindlessHandle;
        MaterialData->SpecularMapBindlessHandle     = SpecularMapBindlessHandle;
        MaterialData->NormalMapBindlessHandle       = NormalMapBindlessHandle;
        MaterialData->DisplacementMapBindlessHandle = DisplacementMapBindlessHandle;
    }

    void CBlinnPhongMapsMaterial::SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms)
    {
        InPrepassUniforms->bHasNormalMap       = NormalMap != nullptr;
        InPrepassUniforms->bHasDisplacementMap = DisplacementMap != nullptr;

        InPrepassUniforms->NormalMapBindlessHandle       = NormalMapBindlessHandle;
        InPrepassUniforms->DisplacementMapBindlessHandle = DisplacementMapBindlessHandle;
    }

    void CBlinnPhongMapsMaterial::InternalSaveToResourceFile(const EFileFormat& InFileFormat)
    {
        FBlinnPhongMapsMaterialDescription BlinnPhongMapsMaterialDescription;
        BlinnPhongMapsMaterialDescription.Id         = AssetId;
        BlinnPhongMapsMaterialDescription.Name       = FDString{ *Name, Name.GetLength() };
        BlinnPhongMapsMaterialDescription.ShaderName = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };

        if (DiffuseMap)
        {
            BlinnPhongMapsMaterialDescription.DiffuseTextureID = DiffuseMap->GetID();
        }
        else
        {
            BlinnPhongMapsMaterialDescription.DiffuseTextureID = sole::INVALID_UUID;
        }

        if (SpecularMap)
        {
            BlinnPhongMapsMaterialDescription.SpecularTextureID = SpecularMap->GetID();
        }
        else
        {
            BlinnPhongMapsMaterialDescription.SpecularColor     = VecToFloat3(SpecularColor);
            BlinnPhongMapsMaterialDescription.SpecularTextureID = sole::INVALID_UUID;
        }

        if (NormalMap)
        {
            BlinnPhongMapsMaterialDescription.NormalTextureID = NormalMap->GetID();
        }
        else
        {
            BlinnPhongMapsMaterialDescription.NormalTextureID = sole::INVALID_UUID;
        }

        if (DisplacementMap)
        {
            BlinnPhongMapsMaterialDescription.DisplacementTextureID = DisplacementMap->GetID();
        }
        else
        {
            BlinnPhongMapsMaterialDescription.DisplacementTextureID = sole::INVALID_UUID;
        }

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(BlinnPhongMapsMaterialDescription, *AssetPath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(BlinnPhongMapsMaterialDescription, *AssetPath);
            break;
        }
    }

    void CBlinnPhongMapsMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();
        if (ImGui::InputInt("Shininess", (int*)&Shininess))
        {
            bMaterialDataDirty = true;
        }

        if (ImGui::DragFloat3("Fallback Specular color", &SpecularColor.r, 0.005, 0, 1))
        {
            bMaterialDataDirty = true;
        }

        ImGui::Text("Diffuse texture:");
        resources::CTextureResource* OldDiffuseMap = DiffuseMap;
        if (ImGuiTextureResourcePicker("blinn_phong_maps_diffuse", &DiffuseMap))
        {
            if (OldDiffuseMap)
            {
                OldDiffuseMap->Release();
            }
            if (DiffuseMap)
            {
                DiffuseMap->Acquire(false, true);
                DiffuseMapBindlessHandle = DiffuseMap->TextureHandle->GetBindlessHandle();
                DiffuseMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("Specular texture:");
        resources::CTextureResource* OldSpecularMap = SpecularMap;
        if (ImGuiTextureResourcePicker("blinn_phong_maps_specular", &SpecularMap))
        {
            if (OldSpecularMap)
            {
                OldSpecularMap->Release();
            }
            if (SpecularMap)
            {
                SpecularMap->Acquire(false, true);
                SpecularMapBindlessHandle = SpecularMap->TextureHandle->GetBindlessHandle();
                SpecularMap->TextureHandle->MakeBindlessResident();
            }
            bMaterialDataDirty = true;
        }

        ImGui::Text("Normal texture:");
        resources::CTextureResource* OldNormalMap = NormalMap;

        if (ImGuiTextureResourcePicker("blinn_phong_maps_normal", &NormalMap))
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
        if (ImGuiTextureResourcePicker("blinn_phong_maps_displacment", &DisplacementMap))
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

    CMaterial* CBlinnPhongMapsMaterial::GetCopy() const
    {
        auto* Copy                          = new CBlinnPhongMapsMaterial{ AssetId, Name, AssetPath, Shader };
        Copy->Shininess                     = Shininess;
        Copy->SpecularColor                 = SpecularColor;
        Copy->DiffuseMap                    = DiffuseMap;
        Copy->SpecularMap                   = SpecularMap;
        Copy->NormalMap                     = NormalMap;
        Copy->DisplacementMap               = DisplacementMap;
        Copy->DiffuseMapBindlessHandle      = DiffuseMapBindlessHandle;
        Copy->SpecularMapBindlessHandle     = SpecularMapBindlessHandle;
        Copy->NormalMapBindlessHandle       = NormalMapBindlessHandle;
        Copy->DisplacementMapBindlessHandle = DisplacementMapBindlessHandle;
        return Copy;
    };

    u16 CBlinnPhongMapsMaterial::GetShaderDataSize() const { return sizeof(FBlinnPhongMapsMaterialData); }

    void CBlinnPhongMapsMaterial::LoadResources()
    {
        CMaterial::LoadResources();

        if (DiffuseMap)
        {
            DiffuseMap->Acquire(false, true);
            if (DiffuseMapBindlessHandle == 0)
            {
                DiffuseMapBindlessHandle = DiffuseMap->TextureHandle->GetBindlessHandle();
                DiffuseMap->TextureHandle->MakeBindlessResident();
            }
        }

        if (SpecularMap)
        {
            SpecularMap->Acquire(false, true);
            if (SpecularMapBindlessHandle == 0)
            {
                SpecularMapBindlessHandle = SpecularMap->TextureHandle->GetBindlessHandle();
                SpecularMap->TextureHandle->MakeBindlessResident();
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
    
    void CBlinnPhongMapsMaterial::UnloadResources()
    {
        if (DiffuseMap)
        {
            DiffuseMap->Release();
            DiffuseMapBindlessHandle = 0;
        }

        if (SpecularMap)
        {
            SpecularMap->Release();
            SpecularMapBindlessHandle = 0;
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