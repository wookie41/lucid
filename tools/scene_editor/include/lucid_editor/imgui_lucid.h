#pragma once

#include "scene/actors/actor.hpp"
#include "scene/actors/actor_enums.hpp"

namespace lucid
{
    namespace resources
    {
        class CTextureResource;
        class CMeshResource;
    } // namespace resources

    namespace scene
    {
        class CMaterial;
        class IActor;
    } // namespace scene

    namespace gpu
    {
        class CShader;
    }
} // namespace lucid
namespace lucid
{
    bool           ImGuiTextureResourcePicker(const char* InLabel, resources::CTextureResource** OutTextureResource);
    void           ImGuiMeshResourcePicker(const char* InLabel, resources::CMeshResource** OutMeshResource);
    void           ImGuiMaterialPicker(const char* InLabel, scene::CMaterial** OutMaterial);
    void           ImGuiShadersPicker(const char* InLabel, gpu::CShader** OutShader);
    void           ImGuiShowMaterialEditor(scene::CMaterial* InMaterial, bool* OutbOpen);
    void           ImGuiActorAssetPicker(const char* InLabel, scene::IActor** OutActor, const scene::EActorType& InTypeFilter);
    glm::vec2      GetMouseScreenSpacePos();
    glm::vec2      GetMouseNDCPos();
    scene::IActor* SceneWindow_GetActorUnderCursor();
    float          SceneWindow_GetDistanceToCameraUnderCursor();

} // namespace lucid
