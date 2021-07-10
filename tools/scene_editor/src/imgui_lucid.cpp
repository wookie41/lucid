#include "devices/gpu/texture.hpp"
#include "devices/gpu/shader.hpp"

#include "resources/mesh_resource.hpp"
#include "resources/texture_resource.hpp"

#include "engine/engine.hpp"

#include "scene/actors/actor_enums.hpp"
#include "scene/material.hpp"

#include "lucid_editor/editor.hpp"
#include "lucid_editor/imgui_lucid.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "scene/renderer.hpp"
#include "scene/world.hpp"

namespace lucid
{
    bool ImGuiTextureResourcePicker(const char* InLabel, resources::CTextureResource** OutTextureResource)
    {
        bool bChanged = false;
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            if (ImGui::Selectable("-- None --", *OutTextureResource == nullptr))
            {
                *OutTextureResource = nullptr;
                bChanged            = true;
            }

            for (int i = 0; i < GEngine.GetTexturesHolder().Length(); ++i)
            {
                resources::CTextureResource* CurrTexture = GEngine.GetTexturesHolder().GetByIndex(i);
                if (gpu::CTexture* ThumbTexture = CurrTexture->GetThumbnail())
                {
                    ThumbTexture->ImGuiDrawToImage({ 16, 16 });
                    ImGui::SameLine();
                }
                if (ImGui::Selectable(*CurrTexture->GetName(), *OutTextureResource == CurrTexture))
                {
                    *OutTextureResource = CurrTexture;
                    bChanged            = true;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutTextureResource == CurrTexture)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
        return bChanged;
    }

    void ImGuiMeshResourcePicker(const char* InLabel, resources::CMeshResource** OutMeshResource)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (int i = 0; i < GEngine.GetMeshesHolder().Length(); ++i)
            {
                resources::CMeshResource* CurrMeshResource = GEngine.GetMeshesHolder().GetByIndex(i);
                if (ImGui::Selectable(*CurrMeshResource->GetName(), *OutMeshResource == CurrMeshResource))
                {
                    *OutMeshResource = CurrMeshResource;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutMeshResource == CurrMeshResource)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    void ImGuiMaterialPicker(const char* InLabel, scene::CMaterial** OutMaterial)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (int i = 0; i < GEngine.GetMaterialsHolder().GetLength(); ++i)
            {
                scene::CMaterial* CurrMaterial = GEngine.GetMaterialsHolder().GetByIndex(i);
                if (ImGui::Selectable(*CurrMaterial->GetName(), *OutMaterial && (*OutMaterial)->GetID() == CurrMaterial->GetID()))
                {
                    *OutMaterial = CurrMaterial;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutMaterial && (*OutMaterial)->GetID() == CurrMaterial->GetID())
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    void ImGuiShadersPicker(const char* InLabel, gpu::CShader** OutShader)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            if (ImGui::Selectable("-- None --", *OutShader == nullptr))
            {
                *OutShader = nullptr;
            }

            const FStringHashMap<gpu::CShader*>& AllShaders = GEngine.GetShadersManager().GetAllShaders();
            for (int i = 0; i < AllShaders.GetLength(); ++i)
            {
                gpu::CShader* CurrShader = AllShaders.Get(i);
                if (ImGui::Selectable(*CurrShader->GetName(), *OutShader == CurrShader))
                {
                    *OutShader = CurrShader;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutShader == CurrShader)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    void ImGuiShowMaterialEditor(scene::CMaterial* InMaterial, bool* OutbOpen)
    {
        ImGui::SetNextWindowSize({ 600, 0 });
        ImGui::Begin("Material editor", OutbOpen);

        if (InMaterial)
        {
            if (ImGui::Button("Save"))
            {
                InMaterial->SaveToResourceFile(EFileFormat::Json);
            }

            ImGui::SameLine();

            if (ImGui::Button("Make default"))
            {
                GEngine.SetDefaultMaterial(InMaterial);
            }

            InMaterial->UIDrawMaterialEditor();
        }
        else
        {
            ImGui::Text("-- No material selected --");
        }

        ImGui::End();
    }

    void ImGuiActorAssetPicker(const char* InLabel, scene::IActor** OutActor, const scene::EActorType& InTypeFilter)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            auto& ActorResources = GEngine.GetActorsResources();
            for (int i = 0; i < ActorResources.GetLength(); ++i)
            {
                scene::IActor* CurrActor = ActorResources.GetByIndex(i);
                if (CurrActor->GetActorType() != InTypeFilter)
                {
                    continue;
                }
                if (ImGui::Selectable(*CurrActor->Name, *OutActor == CurrActor))
                {
                    *OutActor = CurrActor;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutActor == CurrActor)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    glm::vec2 GetMouseScreenSpacePos()
    {
        const ImVec2 MousePositionAbs = ImGui::GetMousePos();
        return { MousePositionAbs.x - GSceneEditorState.SceneWindowPos.x, MousePositionAbs.y - GSceneEditorState.SceneWindowPos.y };
    }

    glm::vec2 GetMouseNDCPos()
    {
        const glm::vec2 MouseScreenSpacePos = GetMouseScreenSpacePos();
        return 2.f *
                 glm::vec2{ MouseScreenSpacePos.x / GSceneEditorState.SceneWindowWidth, 1 - (MouseScreenSpacePos.y / GSceneEditorState.SceneWindowHeight) } -
               1.f;
    }

    template <typename T>
    struct FCachedTextureHitResult
    {
        T    Value{ 0 };
        bool bHit = false;
    };

    template <typename T>
    static void SceneWindow_GetCachedTextureValueUnderCursor(const scene::FCachedTexture<T>& CachedTexture, FCachedTextureHitResult<T>& OutHitResult)
    {
        OutHitResult.bHit = false;

        const glm::vec2 MouseScreenSpacePos = GetMouseScreenSpacePos();

        // Check if we're outside the scene window
        if (MouseScreenSpacePos.x < 0 || MouseScreenSpacePos.x > GSceneEditorState.SceneWindowWidth || MouseScreenSpacePos.y < 0 ||
            MouseScreenSpacePos.y > GSceneEditorState.SceneWindowHeight)
        {
            return;
        }

        // Check if the scene window is not obscured by some other widget
        if (ImGui::GetFocusID() != GSceneEditorState.ImSceneWindow->ID && ImGui::GetFocusID() != GSceneEditorState.SceneDockId)
        {
            return;
        }

        if (IsMouseButtonPressed(LEFT) && GSceneEditorState.World)
        {
            // Adjust mouse postion based on hitmap texture size
            const float RatioX = MouseScreenSpacePos.x / GSceneEditorState.SceneWindowWidth;
            const float RatioY = MouseScreenSpacePos.y / GSceneEditorState.SceneWindowHeight;

            const glm::vec2 AdjustedMousePos = { ((float)CachedTexture.Width) * RatioX, ((float)CachedTexture.Height) * RatioY };

            OutHitResult.bHit  = true;
            OutHitResult.Value = CachedTexture.GetValueAtPosition(AdjustedMousePos);
        }
    }

    scene::IActor* SceneWindow_GetActorUnderCursor()
    {
        static FCachedTextureHitResult<u32> ActorHitResult;
        SceneWindow_GetCachedTextureValueUnderCursor(GEngine.GetRenderer()->GetCachedHitMap(), ActorHitResult);

        if (!ActorHitResult.bHit)
        {
            return nullptr;
        }

        if (scene::IActor* ClickedActor = GSceneEditorState.World->GetActorById(ActorHitResult.Value))
        {
            if (GSceneEditorState.CurrentlySelectedActor == nullptr)
            {
                // Remember the actor that we hit and how far from the camera it was on the z axis
                GSceneEditorState.CurrentlySelectedActor = ClickedActor;
            }
        }
    }

    float SceneWindow_GetDistanceToCameraUnderCursor()
    {
        static FCachedTextureHitResult<float> DistanceHitResult;
        SceneWindow_GetCachedTextureValueUnderCursor(GEngine.GetRenderer()->GetCachedDistanceToCameraMap(), DistanceHitResult);

        if (DistanceHitResult.bHit)
        {
            return DistanceHitResult.Value;
        }

        return -1;
    }

} // namespace lucid