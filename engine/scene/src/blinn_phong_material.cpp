#include "scene/blinn_phong_material.hpp"


#include "engine/engine.hpp"
#include "devices/gpu/shader.hpp"

#include "schemas/binary.hpp"
#include "schemas/json.hpp"

#if DEVELOPMENT
#include "imgui.h"
#include "imgui_lucid.h"
#endif

namespace lucid::scene
{
    static const FSString SHININESS("uMaterialShininess");
    static const FSString DIFFUSE_COLOR("uMaterialDiffuseColor");
    static const FSString SPECULAR_COLOR("uMaterialSpecularColor");

    CBlinnPhongMaterial::CBlinnPhongMaterial(const UUID& InId,
                                             const FDString& InName,
                                             const FDString& InResourcePath,
                                             gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    void CBlinnPhongMaterial::SetupShader(gpu::CShader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        Shader->SetVector(DIFFUSE_COLOR, DiffuseColor);
        Shader->SetVector(SPECULAR_COLOR, SpecularColor);
    };

    CBlinnPhongMaterial* CBlinnPhongMaterial::CreateMaterial(const FBlinnPhongMaterialDescription& Description,
                                                             const FDString& InResourcePath)
    {
        gpu::CShader* Shader = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto* Material = new CBlinnPhongMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->Shininess = Description.Shininess;
        Material->SpecularColor = { Description.SpecularColor[0], Description.SpecularColor[1], Description.SpecularColor[2] };
        Material->DiffuseColor = { Description.DiffuseColor[0], Description.DiffuseColor[1], Description.DiffuseColor[2] };

        return Material;
    }

    void CBlinnPhongMaterial::InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat)
    {
        FBlinnPhongMaterialDescription BlinnPhongMaterialDescription;
        BlinnPhongMaterialDescription.Id = ID;
        BlinnPhongMaterialDescription.Name = FDString{ *Name, Name.GetLength() };
        BlinnPhongMaterialDescription.ShaderName = FDString{ *Shader->GetName(), Shader->GetName().GetLength() };
        BlinnPhongMaterialDescription.Shininess = Shininess;
        BlinnPhongMaterialDescription.DiffuseColor = VecToFloat3(DiffuseColor);
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
        auto* Copy = new CBlinnPhongMaterial { ID, Name, ResourcePath, Shader };
        Copy->Shininess = Shininess;
        Copy->DiffuseColor = DiffuseColor;
        Copy->SpecularColor = SpecularColor;
        return Copy;
    }
    
    /* ---------------------------------------------------------------------------*/

    static const FSString DIFFUSE_MAP("uMaterialDiffuseMap");
    static const FSString SPECULAR_MAP("uMaterialSpecularMap");
    static const FSString NORMAL_MAP("uMaterialNormalMap");
    static const FSString HAS_SPECULAR_MAP("uMaterialHasSpecularMap");
    static const FSString HAS_NORMAL_MAP("uMaterialHasNormalMap");
    static const FSString HAS_DISPLACEMENT_MAP("uMaterialHasDisplacementMap");
    static const FSString DISPLACEMENT_MAP("uMaterialDisplacementMap");

    CBlinnPhongMapsMaterial::CBlinnPhongMapsMaterial(const UUID& InId,
                                                     const FDString& InName,
                                                     const FDString& InResourcePath,
                                                     gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    void CBlinnPhongMapsMaterial::SetupShader(gpu::CShader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        if (DiffuseMap)
        {
            Shader->UseTexture(DIFFUSE_MAP, DiffuseMap->TextureHandle);            
        }

        if (SpecularMap)
        {
            Shader->UseTexture(SPECULAR_MAP, SpecularMap->TextureHandle);
            Shader->SetBool(HAS_SPECULAR_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_SPECULAR_MAP, false);
            Shader->SetVector(SPECULAR_COLOR, SpecularColor);
        }

        if (NormalMap)
        {
            Shader->UseTexture(NORMAL_MAP, NormalMap->TextureHandle);
            Shader->SetBool(HAS_NORMAL_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_NORMAL_MAP, false);
        }

        if (DisplacementMap)
        {
            Shader->UseTexture(DISPLACEMENT_MAP, DisplacementMap->TextureHandle);
            Shader->SetBool(HAS_DISPLACEMENT_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_DISPLACEMENT_MAP, false);
        }
    };

    CBlinnPhongMapsMaterial* CBlinnPhongMapsMaterial::CreateMaterial(const FBlinnPhongMapsMaterialDescription& Description,
                                                                     const FDString& InResourcePath)
    {
        gpu::CShader* Shader = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto* Material = new CBlinnPhongMapsMaterial{ Description.Id, Description.Name, InResourcePath, Shader };

        Material->Shininess = Description.Shininess;
        Material->DiffuseMap = GEngine.GetTexturesHolder().Get(Description.DiffuseTextureID);
        if (Description.SpecularTextureID != sole::INVALID_UUID)
        {
            Material->SpecularMap = GEngine.GetTexturesHolder().Get(Description.SpecularTextureID);
        }
        else
        {
            Material->SpecularColor = Float3ToVec(Description.SpecularColor);
        }

        if (Description.NormalTextureID  != sole::INVALID_UUID)
        {
            Material->NormalMap = GEngine.GetTexturesHolder().Get(Description.NormalTextureID);
        }

        if (Description.DisplacementTextureID != sole::INVALID_UUID)
        {
            Material->DisplacementMap = GEngine.GetTexturesHolder().Get(Description.DisplacementTextureID);
        }

        return Material;
    }

    void CBlinnPhongMapsMaterial::InternalSaveToResourceFile(const EFileFormat& InFileFormat)
    {
        FBlinnPhongMapsMaterialDescription BlinnPhongMapsMaterialDescription;
        BlinnPhongMapsMaterialDescription.Id = ID;
        BlinnPhongMapsMaterialDescription.Name = FDString { *Name, Name.GetLength() };
        BlinnPhongMapsMaterialDescription.ShaderName = FDString { *Shader->GetName(), Shader->GetName().GetLength() };

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
            BlinnPhongMapsMaterialDescription.SpecularColor = VecToFloat3(SpecularColor);
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
        auto* Copy = new CBlinnPhongMapsMaterial { ID, Name, ResourcePath, Shader };
        Copy->Shininess = Shininess;
        Copy->SpecularColor = SpecularColor;
        Copy->DiffuseMap = DiffuseMap;
        Copy->SpecularMap = SpecularMap;
        Copy->NormalMap = NormalMap;
        Copy->DisplacementMap = DisplacementMap;
        return Copy;
    };

} // namespace lucid::scene