#include "scene/actors/actor.hpp"

#include <engine/engine.hpp>


#include "imgui.h"

namespace lucid::scene
{
    void IActor::UIDrawActorDetails()
    {
        ImGui::Text(*Name);

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

    void IActor::SaveToResourceFile(const FString& InActorResourceName)
    {
        FDString ActorResourceFilePath;
        if (InActorResourceName.GetLength())
        {
            ResourceId = sole::uuid4();
            ActorResourceFilePath = SPrintf("assets/actors/%s.asset", *InActorResourceName);

            FActorDatabaseEntry NewEntry;
            NewEntry.ActorId = ResourceId;
            NewEntry.ActorPath = ActorResourceFilePath;

            GEngine.GetActorsDatabase().Entries.push_back(NewEntry);
        }
        else
        {
            assert(ResourcePath.GetLength());
            ActorResourceFilePath = ResourcePath;
        }

        SaveToResourceFile(ActorResourceFilePath);
    }

} // namespace lucid::scene
