#include "scene/flat_material.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const FString COLOR("uMaterial.Color");

    FlatMaterial::FlatMaterial(gpu::CShader* CustomShader) : CMaterial(CustomShader) {}

    void FlatMaterial::SetupShader(gpu::CShader* Shader) { Shader->SetVector(COLOR, Color); }

} // namespace lucid::scene
