#include "scene/flat_material.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const String COLOR("uMaterial.Color");

    FlatMaterial::FlatMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void FlatMaterial::SetupShader(gpu::Shader* Shader) { Shader->SetVector(COLOR, Color); }

} // namespace lucid::scene
