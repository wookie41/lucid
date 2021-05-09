﻿#include "scene/actors/actor.hpp"
#include "engine/engine.hpp"
#include "imgui.h"
#include "imgui_lucid.h"

namespace lucid::scene
{
    void IActor::UIDrawActorDetails()
    {
        ImGui::Text(*Name);

        if (ImGui::Button("Save"))
        {
            SaveToResourceFile();
        }

        // Don't allow do modify translation and similar things on actor assets
        if (ResourceId == sole::INVALID_UUID)
        {
            bBaseActorAssetChanged = false;
            IActor const* OldBaseActor = BaseActorAsset;

            ImGui::Text("Base actor asset:");
            ImGuiActorAssetPicker("Base actor asset", &BaseActorAsset);

            if (BaseActorAsset && BaseActorAsset != OldBaseActor && BaseActorAsset->GetActorType() == GetActorType())
            {
                bBaseActorAssetChanged = true;
            }
            else
            {
                BaseActorAsset = OldBaseActor;
            }

            static glm::vec3 EulerRotation{ glm::degrees(eulerAngles(Transform.Rotation).x),
                                            glm::degrees(eulerAngles(Transform.Rotation).y),
                                            glm::degrees(eulerAngles(Transform.Rotation).z) };

            ImGui::DragFloat3("Translation (x, y, z)", &Transform.Translation.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
            ImGui::DragFloat3("Scale (x, y, z)", &Transform.Scale.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
            ImGui::DragFloat3("Rotation", &EulerRotation.x, 0.1f, -360, 360);
            ImGui::Checkbox("Visible", &bVisible);
            ImGui::Spacing();

            Transform.Rotation =
              glm::quat({ glm::radians(EulerRotation.x), glm::radians(EulerRotation.y), glm::radians(EulerRotation.z) });
        }
    }

    void IActor::SaveToResourceFile()
    {
        if (BaseActorAsset)
        {
            auto* NewActorAsset = CreateActorAsset(Name.GetCopy());
            NewActorAsset->ResourceId = sole::uuid4();
            NewActorAsset->ResourcePath = SPrintf("assets/actors/%s.asset", *Name);

            FActorDatabaseEntry NewEntry;
            NewEntry.ActorId = NewActorAsset->ResourceId;
            NewEntry.ActorPath = NewActorAsset->ResourcePath;

            GEngine.AddActorAsset(NewActorAsset);

            BaseActorAsset = NewActorAsset;
            NewActorAsset->_SaveToResourceFile(NewActorAsset->ResourcePath);
        }
        else
        {
            assert(ResourcePath.GetLength());
            _SaveToResourceFile(ResourcePath);
        }
    }

} // namespace lucid::scene
