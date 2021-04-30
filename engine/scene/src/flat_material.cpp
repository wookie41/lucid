#include "scene/flat_material.hpp"

#include "engine/engine.hpp"
#include "devices/gpu/shader.hpp"

namespace lucid::scene
{
    static const FSString COLOR("uFlatColor");

    FlatMaterial::FlatMaterial(const FString& InName, gpu::CShader* InShader) : CMaterial(InName, InShader) {}

    void FlatMaterial::SetupShader(gpu::CShader* Shader) { Shader->SetVector(COLOR, Color); }

    FlatMaterial* CreateFlatMaterial(const FFlatMaterialDescription& Description)
    {
        gpu::CShader* Shader = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);        
        auto* Material = new FlatMaterial { Description.Name, Shader };
        Material->Color = { Description.Color[0], Description.Color[1], Description.Color[2], Description.Color[3] };
        return Material;
    }
} // namespace lucid::scene
