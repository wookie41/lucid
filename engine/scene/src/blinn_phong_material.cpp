#include "scene/blinn_phong_material.hpp"

#include "engine/engine.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const FSString SHININESS("uMaterialShininess");
    static const FSString DIFFUSE_COLOR("uMaterialDiffuseColor");
    static const FSString SPECULAR_COLOR("uMaterialSpecularColor");

    CBlinnPhongMaterial::CBlinnPhongMaterial(const FString& InName, gpu::CShader* InShader) : CMaterial(InName, InShader) {}

    void CBlinnPhongMaterial::SetupShader(gpu::CShader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        Shader->SetVector(DIFFUSE_COLOR, DiffuseColor);
        Shader->SetVector(DIFFUSE_COLOR, SpecularColor);
    };

        CBlinnPhongMaterial* CreateBlinnPhongMaterial(const FBlinnPhongMaterialDescription& Description)
        {
            gpu::CShader* Shader = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);        
            auto* Material = new CBlinnPhongMaterial { Description.Name, Shader };
    
            Material->Shininess = Description.Shininess;
            Material->SpecularColor = { Description.SpecularColor[0], Description.SpecularColor[1], Description.SpecularColor[2] };
            Material->DiffuseColor = { Description.DiffuseColor[0], Description.DiffuseColor[1], Description.DiffuseColor[2] };
    
            GEngine.GetMaterialsHolder().Add(*Material->GetName(), Material);
            return Material;
        }

    /* ---------------------------------------------------------------------------*/

    static const FSString DIFFUSE_MAP("uMaterialDiffuseMap");
    static const FSString SPECULAR_MAP("uMaterialSpecularMap");
    static const FSString NORMAL_MAP("uMaterialNormalMap");
    static const FSString HAS_SPECULAR_MAP("uMaterialHasSpecularMap");
    static const FSString HAS_NORMAL_MAP("uMaterialHasNormalMap");
    static const FSString HAS_DISPLACEMENT_MAP("uMaterialHasDisplacementMap");
    static const FSString DISPLACEMENT_MAP("uMaterialDisplacementMap");

    CBlinnPhongMapsMaterial::CBlinnPhongMapsMaterial(const FString& InName, gpu::CShader* InShader) : CMaterial(InName, InShader) {}

    void CBlinnPhongMapsMaterial::SetupShader(gpu::CShader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        Shader->UseTexture(DIFFUSE_MAP, DiffuseMap);

        if (SpecularMap != nullptr)
        {
            Shader->UseTexture(SPECULAR_MAP, SpecularMap);
            Shader->SetBool(HAS_SPECULAR_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_SPECULAR_MAP, false);
            Shader->SetVector(SPECULAR_COLOR, SpecularColor);
        }

        if (NormalMap != nullptr)
        {
            Shader->UseTexture(NORMAL_MAP, NormalMap);
            Shader->SetBool(HAS_NORMAL_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_NORMAL_MAP, false);
        }

        if (DisplacementMap != nullptr)
        {
            Shader->UseTexture(DISPLACEMENT_MAP, DisplacementMap);
            Shader->SetBool(HAS_DISPLACEMENT_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_DISPLACEMENT_MAP, false);
        }
    };

    CBlinnPhongMapsMaterial* CreateBlinnPhongMapsMaterial(const FBlinnPhongMapsMaterialDescription& Description)
    {
        gpu::CShader* Shader = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);        
        auto* Material = new CBlinnPhongMapsMaterial { Description.Name, Shader };

        Material->DiffuseMap = GEngine.GetTexturesHolder().Get(*Description.DiffuseTextureName)->TextureHandle;
        Material->SpecularMap = GEngine.GetTexturesHolder().Get(*Description.SpecularTextureName)->TextureHandle;
        Material->NormalMap = GEngine.GetTexturesHolder().Get(*Description.NormalTextureName)->TextureHandle;
        Material->DisplacementMap = GEngine.GetTexturesHolder().Get(*Description.DisplacementTextureName)->TextureHandle;
        Material->SpecularColor = { Description.SpecularColor[0], Description.SpecularColor[1], Description.SpecularColor[2] };

        GEngine.GetMaterialsHolder().Add(*Material->GetName(), Material);
        return Material;
    }

} // namespace lucid::scene