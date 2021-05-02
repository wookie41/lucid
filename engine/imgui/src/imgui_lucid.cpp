#include "imgui_lucid.h"

#include "resources/mesh_resource.hpp"
#include "resources/texture_resource.hpp"
#include "engine/engine.hpp"
#include "scene/material.hpp"

#include "imgui.h"

namespace lucid
{
    void ImGuiTextureResourcePicker(const char* InLabel, resources::CTextureResource** OutTextureResource)
    {
        if (ImGui::BeginListBox(InLabel, ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            if(ImGui::Selectable("-- None --", *OutTextureResource == nullptr))
            {
                *OutTextureResource = nullptr;
            }

            for (int i = 0; i < GEngine.GetTexturesHolder().Length(); ++i)
            {
                resources::CTextureResource* CurrTexture = GEngine.GetTexturesHolder().GetByIndex(i);
                if(ImGui::Selectable(*CurrTexture->GetName(), *OutTextureResource == CurrTexture))
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
            if(ImGui::Selectable("-- None --", *OutMeshResource == nullptr))
            {
                *OutMeshResource = nullptr;
            }
            
            for (int i = 0; i < GEngine.GetMeshesHolder().Length(); ++i)
            {
                resources::CMeshResource* CurrMeshResource = GEngine.GetMeshesHolder().GetByIndex(i);
                if(ImGui::Selectable(*CurrMeshResource->GetName(), *OutMeshResource == CurrMeshResource))
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
            if(ImGui::Selectable("-- None --", *OutMaterial == nullptr))
            {
                *OutMaterial = nullptr;
            }
            
            for (int i = 0; i < GEngine.GetMaterialsHolder().GetLength(); ++i)
            {
                scene::CMaterial* CurrMaterial = GEngine.GetMaterialsHolder().Get(i);
                if(ImGui::Selectable(*CurrMaterial->GetName(), *OutMaterial == CurrMaterial))
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
}