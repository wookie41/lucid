#include "imgui_lucid.h"

#include "resources/mesh_resource.hpp"
#include "resources/texture_resource.hpp"
#include "engine/engine.hpp"
#include "scene/material.hpp"

#include "devices/gpu/shader.hpp"

#include "imgui.h"

namespace lucid
{
    void ImGuiTextureResourcePicker(const char* InLabel, resources::CTextureResource** OutTextureResource)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            if (ImGui::Selectable("-- None --", *OutTextureResource == nullptr))
            {
                *OutTextureResource = nullptr;
            }

            for (int i = 0; i < GEngine.GetTexturesHolder().Length(); ++i)
            {
                resources::CTextureResource* CurrTexture = GEngine.GetTexturesHolder().GetByIndex(i);
                if (ImGui::Selectable(*CurrTexture->GetName(), *OutTextureResource == CurrTexture))
                {
                    *OutTextureResource = CurrTexture;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutTextureResource == CurrTexture)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    void ImGuiMeshResourcePicker(const char* InLabel, resources::CMeshResource** OutMeshResource)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            if (ImGui::Selectable("-- None --", *OutMeshResource == nullptr))
            {
                *OutMeshResource = nullptr;
            }

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
            if (ImGui::Selectable("-- None --", *OutMaterial == nullptr))
            {
                *OutMaterial = nullptr;
            }

            for (int i = 0; i < GEngine.GetMaterialsHolder().GetLength(); ++i)
            {
                scene::CMaterial* CurrMaterial = GEngine.GetMaterialsHolder().GetByIndex(i);
                if (ImGui::Selectable(*CurrMaterial->GetName(), *OutMaterial == CurrMaterial))
                {
                    *OutMaterial = CurrMaterial;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (*OutMaterial == CurrMaterial)
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
            InMaterial->UIDrawMaterialEditor();
        }
        else
        {
            ImGui::Text("-- No material selected --");
        }
        ImGui::End();
    }

} // namespace lucid