#include "scene/blinn_phong_material.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const String SHININESS("uMaterial.Shininess");
    static const String DIFFUSE_COLOR("uMaterial.DiffuseColor");
    static const String SPECULAR_COLOR("uMaterial.SpecularColor");

    BlinnPhongMaterial::BlinnPhongMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMaterial::SetupShader(gpu::Shader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        Shader->SetVector(DIFFUSE_COLOR, DiffuseColor);
        Shader->SetVector(DIFFUSE_COLOR, SpecularColor);
    };

    /* ---------------------------------------------------------------------------*/

    static const String DIFFUSE_MAP("uMaterial.DiffuseMap");
    static const String SPECULAR_MAP("uMaterial.SpecularMap");
    static const String NORMAL_MAP("uMaterial.NormalMap");
    static const String HAS_SPECULAR_MAP("uMaterial.HasSpecularMap");
    static const String HAS_NORMAL_MAP("uMaterial.HasNormalMap");

    BlinnPhongMapsMaterial::BlinnPhongMapsMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMapsMaterial::SetupShader(gpu::Shader* Shader)
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
    };
} // namespace lucid::scene