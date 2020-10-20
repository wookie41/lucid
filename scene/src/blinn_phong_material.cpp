#include "scene/blinn_phong_material.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const String SHININESS("uShininess");
    static const String DIFFUSE_COLOR("uDiffuseColor");
    static const String SPECULAR_COLOR("uSpecularColor");

    BlinnPhongMaterial::BlinnPhongMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMaterial::SetupShader(gpu::Shader* Shader)
    {
        ReteriveShaderUniformsIds(Shader);

        Shader->SetFloat(shininessUniformId, Shininess);
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

    static const String DIFFUSE_MAP("uDiffuseMap");
    static const String SPECULAR_MAP("uSpecularMap");
    static const String NORMAL_MAP("uNormalMap");

    BlinnPhongMapsMaterial::BlinnPhongMapsMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMapsMaterial::SetupShader(gpu::Shader* Shader)
    {
        ReteriveShaderUniformsIds(Shader);

        Shader->SetFloat(shininessUniformId, Shininess);
        Shader->UseTexture(diffuseMapUniformId, DiffuseMap);
        Shader->UseTexture(specularMapUniformId, SpecularMap);
        Shader->UseTexture(normalMapUniformId, NormalMap);
    };

    void BlinnPhongMapsMaterial::ReteriveShaderUniformsIds(gpu::Shader* Shader)
    {
        if (Shader == cachedShader)
        {
            return;
        }

        shininessUniformId = Shader->GetTextureId(SHININESS);
        diffuseMapUniformId = Shader->GetTextureId(DIFFUSE_MAP);
        specularMapUniformId = Shader->GetTextureId(SPECULAR_MAP);
        normalMapUniformId = Shader->GetTextureId(NORMAL_MAP);

        cachedShader = Shader;
    }

} // namespace lucid::scene