#include "scene/actors/actor.hpp"

#include "imgui.h"

namespace lucid::scene
{
    void IActor::UIDrawActorDetails()
    {
        ImGui::Text("Actor:");

        if (ImGui::Button("Save"))
        {
            SaveToResourceFile();
        }
        
        ImGui::DragFloat3("Translation", &Transform.Translation.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
        ImGui::DragFloat3("Scale", &Transform.Scale.r, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
        ImGui::DragFloat4("Rotation (Quat)", &Transform.Rotation.x, 0.005f, -1, 1);
        ImGui::Checkbox("Visible", &bVisible);
        ImGui::Spacing();
    }
}
