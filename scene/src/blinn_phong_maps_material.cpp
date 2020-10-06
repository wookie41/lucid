#include "scene/blinn_phong_maps_material.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const String SHININESS("uShininess");
    static const String DIFFUSE_MAP("uDiffuseMap");
    static const String SPECULAR_MAP("uSpecularMap");

    BlinnPhongMapsMaterial::BlinnPhongMapsMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMapsMaterial::SetupShader(gpu::Shader* Shader)
    {
        //@TODO replace with default
        assert(DiffuseMap && SpecularMap);
        ReteriveShaderUniformsIds(Shader);

        Shader->SetFloat(shininessUniformId, Shininess);
        Shader->UseTexture(diffuseMapUniformId, DiffuseMap);
        Shader->UseTexture(specularMapUniformId, SpecularMap);
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

        cachedShader = Shader;
    }
} // namespace lucid::scene