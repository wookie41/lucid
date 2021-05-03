#include "scene/flat_material.hpp"


#if DEVELOPMENT
#include "imgui.h"
#endif

#include "devices/gpu/shader.hpp"
#include "engine/engine.hpp"

#include "schemas/json.hpp"
#include "schemas/binary.hpp"

namespace lucid::scene
{
    static const FSString COLOR("uFlatColor");

    CFlatMaterial::CFlatMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader)
    : CMaterial(InId, InName, InResourcePath, InShader)
    {
    }

    void CFlatMaterial::SetupShader(gpu::CShader* Shader) { Shader->SetVector(COLOR, Color); }

    CFlatMaterial* CFlatMaterial::CreateMaterial(const FFlatMaterialDescription& Description, const FDString& InResourcePath)
    {
        gpu::CShader* Shader = GEngine.GetShadersManager().GetShaderByName(Description.ShaderName);
        auto* Material = new CFlatMaterial{ Description.Id, Description.Name, InResourcePath, Shader };
        Material->Color = Float4ToVec(Description.Color);
        return Material;
    }

    void CFlatMaterial::SaveToResourceFile(const lucid::EFileFormat& InFileFormat)
    {
        FFlatMaterialDescription FlatMaterialDescription;
        FlatMaterialDescription.Id = ID;
        FlatMaterialDescription.Name = FDString { *Name };
        FlatMaterialDescription.ShaderName = FDString { *Shader->GetName() };
        FlatMaterialDescription.Color = VecToFloat4(Color);

        switch (InFileFormat)
        {
        case EFileFormat::Binary:
            WriteToBinaryFile(FlatMaterialDescription, *ResourcePath);
            break;
        case EFileFormat::Json:
            WriteToJSONFile(FlatMaterialDescription, *ResourcePath);
            break;
        }

        GEngine.GetMaterialDatabase().FlatMaterials.push_back({ResourcePath, InFileFormat, GEngine.GetDefaultMaterial() == this});
    }

#if DEVELOPMENT
     void CFlatMaterial::UIDrawMaterialEditor()
    {
        ImGui::Text("Flat material: %s", *Name);
        ImGui::DragFloat4("Color", &Color.r, 0.005, 0, 1);
    }
#endif

} // namespace lucid::scene
