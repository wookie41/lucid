#include "scene/blinn_phong_material.hpp"

#include "engine/engine.hpp"

#include "devices/gpu/shader.hpp"
#include "devices/gpu/fence.hpp"

#include "scene/forward_renderer.hpp"

#include "schemas/binary.hpp"
#include "schemas/json.hpp"

#if DEVELOPMENT
#include "imgui.h"
#include "imgui_lucid.h"
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

    void CBlinnPhongMaterial::SetupShaderBuffers(char* InMaterialDataPtr, u64* InBindlessTexturesArrayPtr, u32& OutMaterialDataSize, u32& BindlessTexturesSize) {}

    void CBlinnPhongMaterial::SetupPrepassShaderBuffers(FForwardPrepassUniforms* InPrepassUniforms, u64* InBindlessTexturesArrayPtr)
    {
        InPrepassUniforms->bHasNormalMap       = false;
        InPrepassUniforms->bHasDisplacementMap = false;
    }

    void CBlinnPhongMaterial::InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat)
    {
        FBlinnPhongMaterialDescription BlinnPhongMaterialDescription;
        BlinnPhongMaterialDescription.Id            = ID;
        BlinnPhongMaterialDescription.Name          = FDString{ *Name, Name.GetLength() };
        BlinnPhongMaterialDescription.ShaderName    = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };
        BlinnPhongMaterialDescription.Shininess     = Shininess;
        BlinnPhongMaterialDescription.DiffuseColor  = VecToFloat3(DiffuseColor);
        BlinnPhongMaterialDescription.SpecularColor = VecToFloat3(SpecularColor);

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(BlinnPhongMaterialDescription, *ResourcePath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(BlinnPhongMaterialDescription, *ResourcePath);
            break;
        }
    }

    void CBlinnPhongMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();
        ImGui::InputInt("Shininess", (int*)&Shininess);
        ImGui::DragFloat3("Diffuse color", &DiffuseColor.r, 0.005, 0, 1);
        ImGui::DragFloat3("Specular color", &SpecularColor.r, 0.005, 0, 1);
    }

    CMaterial* CBlinnPhongMaterial::GetCopy() const
    {
        auto* Copy          = new CBlinnPhongMaterial{ ID, Name, ResourcePath, Shader };
        Copy->Shininess     = Shininess;
        Copy->DiffuseColor  = DiffuseColor;
        Copy->SpecularColor = SpecularColor;
        return Copy;
    }

    u16 CBlinnPhongMaterial::GetShaderDataSize() const
    {
        constexpr u16 DataSize = sizeof(Shininess) + sizeof(DiffuseColor) + sizeof(SpecularColor);
        return DataSize;
    }

    /* ---------------------------------------------------------------------------*/

    static const FSString DIFFUSE_MAP("uMaterialDiffuseMap");
    static const FSString SPECULAR_MAP("uMaterialSpecularMap");
    static const FSString NORMAL_MAP("uMaterialNormalMap");
    static const FSString HAS_SPECULAR_MAP("uMaterialHasSpecularMap");
    static const FSString HAS_NORMAL_MAP("uMaterialHasNormalMap");
    static const FSString HAS_DISPLACEMENT_MAP("uMaterialHasDisplacementMap");
    static const FSString DISPLACEMENT_MAP("uMaterialDisplacementMap");

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

    void CBlinnPhongMapsMaterial::SetupShaderBuffers(char* InMaterialDataPtr, u64* InBindlessTexturesArrayPtr, u32& OutMaterialDataSize, u32& BindlessTexturesSize) {}

    void CBlinnPhongMapsMaterial::SetupPrepassShaderBuffers(FForwardPrepassUniforms* InPrepassUniforms, u64* InBindlessTexturesArrayPtr)
    {
        InPrepassUniforms->bHasNormalMap = NormalMap != nullptr;
        *InBindlessTexturesArrayPtr      = NormalMapBindlessHandle;

        InPrepassUniforms->bHasDisplacementMap = DisplacementMap != nullptr;
        *(InBindlessTexturesArrayPtr + 1)      = DisplacementMapBindlessHandle;
    }

    void CBlinnPhongMapsMaterial::InternalSaveToResourceFile(const EFileFormat& InFileFormat)
    {
        FBlinnPhongMapsMaterialDescription BlinnPhongMapsMaterialDescription;
        BlinnPhongMapsMaterialDescription.Id         = ID;
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
            WriteToBinaryFile(BlinnPhongMapsMaterialDescription, *ResourcePath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(BlinnPhongMapsMaterialDescription, *ResourcePath);
            break;
        }
    }

    void CBlinnPhongMapsMaterial::UIDrawMaterialEditor()
    {
        CMaterial::UIDrawMaterialEditor();
        ImGui::InputInt("Shininess", (int*)&Shininess);
        ImGui::DragFloat3("Fallback Specular color", &SpecularColor.r, 0.005, 0, 1);

        ImGui::Text("Diffuse texture:");
        ImGuiTextureResourcePicker("blinn_phong_maps_diffuse", &DiffuseMap);

        ImGui::Text("Specular texture:");
        ImGuiTextureResourcePicker("blinn_phong_maps_specular", &SpecularMap);

        ImGui::Text("Normal texture:");
        ImGuiTextureResourcePicker("blinn_phong_maps_normal", &NormalMap);

        ImGui::Text("Displacement texture:");
        ImGuiTextureResourcePicker("blinn_phong_maps_displacment", &DisplacementMap);
    }

    CMaterial* CBlinnPhongMapsMaterial::GetCopy() const
    {
        auto* Copy            = new CBlinnPhongMapsMaterial{ ID, Name, ResourcePath, Shader };
        Copy->Shininess       = Shininess;
        Copy->SpecularColor   = SpecularColor;
        Copy->DiffuseMap      = DiffuseMap;
        Copy->SpecularMap     = SpecularMap;
        Copy->NormalMap       = NormalMap;
        Copy->DisplacementMap = DisplacementMap;
        return Copy;
    };

    u16 CBlinnPhongMapsMaterial::GetShaderDataSize() const
    {
        constexpr u16 DataSize = sizeof(Shininess) + sizeof(u64) + // diffuse map handle
                                 sizeof(bool) + // normal map flag + handle
                                 sizeof(u64) + sizeof(bool) + // displacement map flag + handle
                                 sizeof(u64) + sizeof(SpecularColor);
        return DataSize;
    }

    void CBlinnPhongMapsMaterial::LoadResources()
    {
        if (DiffuseMap)
        {
            DiffuseMap->Acquire(false, true);
            DiffuseMapBindlessHandle = DiffuseMap->TextureHandle->GetBindlessHandle();
            DiffuseMap->TextureHandle->MakeBindlessResident();
        }

        if (SpecularMap)
        {
            SpecularMap->Acquire(false, true);
            SpecularMapBindlessHandle = SpecularMap->TextureHandle->GetBindlessHandle();
            SpecularMap->TextureHandle->MakeBindlessResident();
        }

        if (NormalMap)
        {
            NormalMap->Acquire(false, true);
            NormalMapBindlessHandle = NormalMap->TextureHandle->GetBindlessHandle();
            NormalMap->TextureHandle->MakeBindlessResident();
        }

        if (DisplacementMap)
        {
            DisplacementMap->Acquire(false, true);
            DisplacementMapBindlessHandle = DisplacementMap->TextureHandle->GetBindlessHandle();
            DisplacementMap->TextureHandle->MakeBindlessResident();
        }
    }
    void CBlinnPhongMapsMaterial::UnloadResources()
    {
        if (DiffuseMap)
        {
            DiffuseMap->Release();
            DiffuseMapBindlessHandle = 0;
            DiffuseMap->TextureHandle->MakeBindlessNonResident();
        }

        if (SpecularMap)
        {
            SpecularMap->Release();
            SpecularMapBindlessHandle = 0;
            SpecularMap->TextureHandle->MakeBindlessNonResident();
        }

        if (NormalMap)
        {
            NormalMap->Release();
            NormalMapBindlessHandle = 0;
            NormalMap->TextureHandle->MakeBindlessNonResident();
        }

        if (DisplacementMap)
        {
            DisplacementMap->Release();
            DisplacementMapBindlessHandle = 0;
            DisplacementMap->TextureHandle->MakeBindlessNonResident();
        }
    }
} // namespace lucid::scene