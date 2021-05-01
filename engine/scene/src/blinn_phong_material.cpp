#include "scene/blinn_phong_material.hpp"

#include "engine/engine.hpp"
#include "devices/gpu/shader.hpp"

#include "schemas/binary.hpp"
#include "schemas/json.hpp"

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
        Shader->SetVector(DIFFUSE_COLOR, SpecularColor);
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

    void CBlinnPhongMaterial::SaveToResourceFile(const lucid::EFileFormat& InFileFormat) const
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
        Shader->UseTexture(DIFFUSE_MAP, DiffuseMap->TextureHandle);

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
        Material->DiffuseMap = GEngine.GetTexturesHolder().Get(*Description.DiffuseTextureName);
        if (Description.SpecularTextureName.GetLength())
        {
            Material->SpecularMap = GEngine.GetTexturesHolder().Get(*Description.SpecularTextureName);
        }
        else
        {
            Material->SpecularColor = Float3ToVec(Description.SpecularColor);
        }

        if (Description.NormalTextureName.GetLength())
        {
            Material->NormalMap = GEngine.GetTexturesHolder().Get(*Description.NormalTextureName);
        }

        if (Description.DisplacementTextureName.GetLength())
        {
            Material->DisplacementMap = GEngine.GetTexturesHolder().Get(*Description.DisplacementTextureName);
        }

        return Material;
    }

    void CBlinnPhongMapsMaterial::SaveToResourceFile(const lucid::EFileFormat& InFileFormat) const
    {
        FBlinnPhongMapsMaterialDescription BlinnPhongMapsMaterialDescription;
        BlinnPhongMapsMaterialDescription.Id = ID;
        BlinnPhongMapsMaterialDescription.Name = FDString { *Name, Name.GetLength() };
        BlinnPhongMapsMaterialDescription.ShaderName = FDString { *Shader->GetName(), Shader->GetName().GetLength() };

        if (DiffuseMap)
        {
            BlinnPhongMapsMaterialDescription.DiffuseTextureName = FDString { *DiffuseMap->GetName(), DiffuseMap->GetName().GetLength() };
        }

        if (SpecularMap)
        {
            BlinnPhongMapsMaterialDescription.SpecularTextureName = FDString { *SpecularMap->GetName(), SpecularMap->GetName().GetLength() };
        }
        else
        {
            BlinnPhongMapsMaterialDescription.SpecularColor = VecToFloat3(SpecularColor);
        }

        if (NormalMap)
        {
            BlinnPhongMapsMaterialDescription.NormalTextureName = FDString { *NormalMap->GetName(), NormalMap->GetName().GetLength() };
        }

        if (DisplacementMap)
        {
            BlinnPhongMapsMaterialDescription.DisplacementTextureName = FDString { *DisplacementMap->GetName(), DisplacementMap->GetName().GetLength() };
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
} // namespace lucid::scene