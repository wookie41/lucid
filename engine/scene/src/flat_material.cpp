#include "scene/flat_material.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const FSString COLOR("uFlatColor");

    FlatMaterial::FlatMaterial(gpu::CShader* InShader) : CMaterial(InShader) {}

    void FlatMaterial::SetupShader(gpu::CShader* Shader) { Shader->SetVector(COLOR, Color); }

} // namespace lucid::scene
