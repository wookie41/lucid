﻿#pragma once

#include "scene/actors/actor_enums.hpp"

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
        class IActor;
    }

    namespace gpu
    {
        class CShader;
    }
}
namespace lucid
{
    bool ImGuiTextureResourcePicker(const char* InLabel, resources::CTextureResource** OutTextureResource);
    void ImGuiMeshResourcePicker(const char* InLabel, resources::CMeshResource** OutMeshResource);
    void ImGuiMaterialPicker(const char* InLabel, scene::CMaterial** OutMaterial);
    void ImGuiShadersPicker(const char* InLabel, gpu::CShader** OutShader);
    void ImGuiShowMaterialEditor(scene::CMaterial* InMaterial, bool* OutbOpen);
    void ImGuiActorAssetPicker(const char* InLabel, scene::IActor** OutActor, const scene::EActorType& InTypeFilter);
}
