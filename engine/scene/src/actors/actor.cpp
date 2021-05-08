#include "scene/actors/actor.hpp"
#include "engine/engine.hpp"
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

        // Don't allow do modify translation and similar things on actor resources
        if (!ResourcePath.GetLength())
        {
            static glm::vec3 EulerRotation { glm::degrees(eulerAngles(Transform.Rotation).x), glm::degrees(eulerAngles(Transform.Rotation).y), glm::degrees(eulerAngles(Transform.Rotation).z) };
            
            ImGui::DragFloat3("Translation (x, y, z)", &Transform.Translation.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
            ImGui::DragFloat3("Scale (x, y, z)", &Transform.Scale.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
            ImGui::DragFloat3("Rotation", &EulerRotation.x, 0.1f, -360, 360);
            ImGui::Checkbox("Visible", &bVisible);
            ImGui::Spacing();

            Transform.Rotation = glm::quat({ glm::radians(EulerRotation.x), glm::radians(EulerRotation.y), glm::radians(EulerRotation.z) });
        }        
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

        _SaveToResourceFile(ActorResourceFilePath);
    }

} // namespace lucid::scene
