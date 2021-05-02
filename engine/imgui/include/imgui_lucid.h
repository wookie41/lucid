#pragma once

namespace lucid
{
    namespace resources
    {
        class CTextureResource;
        class CMeshResource;        
    }

    namespace scene
    {
        class CMaterial;        
    }
}
namespace lucid
{
    void ImGuiTextureResourcePicker(const char* InLabel, resources::CTextureResource** OutTextureResource);
    void ImGuiMeshResourcePicker(const char* InLabel, resources::CMeshResource** OutMeshResource);
    void ImGuiMaterialPicker(const char* InLabel, scene::CMaterial** OutMaterial);
}
