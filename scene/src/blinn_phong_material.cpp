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
        Shader->SetFloat(SHININESS, Shininess);
        Shader->SetVector(DIFFUSE_COLOR, DiffuseColor);
        Shader->SetVector(SPECULAR_COLOR, SpecularColor);
    };
} // namespace lucid::scene