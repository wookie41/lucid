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
        ReteriveShaderUniformsIds(Shader);

        Shader->SetInt(shininessUniformId, Shininess);
        Shader->SetVector(diffuseColorUniformId, DiffuseColor);
        Shader->SetVector(specularColorUniformId, SpecularColor);
    };

    void BlinnPhongMaterial::ReteriveShaderUniformsIds(gpu::Shader* Shader)
    {
        if (Shader == cachedShader)
        {
            return;
        }

        shininessUniformId = Shader->GetIdForUniform(SHININESS);
        diffuseColorUniformId = Shader->GetIdForUniform(DIFFUSE_COLOR);
        specularColorUniformId = Shader->GetIdForUniform(SPECULAR_COLOR);

        cachedShader = Shader;
    }

    /* ---------------------------------------------------------------------------*/

    static const String DIFFUSE_MAP("uMaterial.DiffuseMap");
    static const String SPECULAR_MAP("uMaterial.SpecularMap");
    static const String NORMAL_MAP("uMaterial.NormalMap");
    static const String HAS_SPECULAR_MAP("uMaterial.HasSpecularMap");
    static const String HAS_NORMAL_MAP("uMaterial.HasNormalMap");

    BlinnPhongMapsMaterial::BlinnPhongMapsMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMapsMaterial::SetupShader(gpu::Shader* Shader)
    {
        ReteriveShaderUniformsIds(Shader);

        Shader->SetInt(shininessUniformId, Shininess);
        Shader->UseTexture(diffuseMapUniformId, DiffuseMap);

        if (SpecularMap != nullptr)
        {
            Shader->UseTexture(specularMapUniformId, SpecularMap);
            Shader->SetBool(hasSpecularMapUniformId, true);
        }
        else
        {
            Shader->SetBool(hasSpecularMapUniformId, false);
        }

        if (NormalMap != nullptr)
        {
            Shader->UseTexture(normalMapUniformId, NormalMap);
            Shader->SetBool(hasNormalMapUniformId, true);
        }
        else
        {
            Shader->SetBool(hasNormalMapUniformId, false);
        }
    };

    void BlinnPhongMapsMaterial::ReteriveShaderUniformsIds(gpu::Shader* Shader)
    {
        if (Shader == cachedShader)
        {
            return;
        }

        shininessUniformId = Shader->GetIdForUniform(SHININESS);
        diffuseMapUniformId = Shader->GetTextureId(DIFFUSE_MAP);
        specularMapUniformId = Shader->GetTextureId(SPECULAR_MAP);
        normalMapUniformId = Shader->GetTextureId(NORMAL_MAP);
        hasSpecularMapUniformId = Shader->GetIdForUniform(HAS_SPECULAR_MAP);
        hasNormalMapUniformId = Shader->GetIdForUniform(HAS_NORMAL_MAP);

        cachedShader = Shader;
    }

} // namespace lucid::scene