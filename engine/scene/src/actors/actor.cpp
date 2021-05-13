#include "scene/actors/actor.hpp"

#include "scene/camera.hpp"
#include "engine/engine.hpp"

#include "imgui.h"
#include "ImGuizmo.h"
#include "imgui_lucid.h"

#include <sole/sole.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "platform/input.hpp"

namespace lucid::scene

{
    void IActor::UIDrawActorDetails()
    {
        if (ImGui::Button("Save"))
        {
            SaveToResourceFile();
        }

        if (ImGui::CollapsingHeader("Actor"))
        {
            static char RenameBuffer[256];

            if (bRenaming)
            {
                ImGui::InputText("##Rename Actor", RenameBuffer, 255);
                ImGui::SameLine();
                if (ImGui::Button("Rename"))
                {
                    Name.ReplaceWithBuffer(RenameBuffer);
                    bRenaming = false;
                }
            }
            else
            {
                ImGui::Text(ResourceId == sole::INVALID_UUID ? "Actor name: %s" : "Actor asset name: %s", *Name);
                ImGui::SameLine();
                if (ImGui::Button("Rename"))
                {
                    Name.CopyToBuffer(RenameBuffer);
                    bRenaming = true;
                }
            }

            // Don't allow do modify translation and similar things on actor assets
            if (ResourceId == sole::INVALID_UUID)
            {
                PrevBaseActorAsset = nullptr;
                IActor* OldBaseActor = BaseActorAsset;

                if (BaseActorAsset)
                {
                    ImGui::Text("Base actor asset:");
                    ImGuiActorAssetPicker("Base actor asset", &BaseActorAsset);

                    if (BaseActorAsset && BaseActorAsset != OldBaseActor && BaseActorAsset->GetActorType() == OldBaseActor->GetActorType())
                    {
                        PrevBaseActorAsset = OldBaseActor;
                        if (!BaseActorAsset->bAssetLoaded)
                        {
                            BaseActorAsset->LoadAsset();
                            BaseActorAsset->bAssetLoaded = true;
                        }
                    }
                    else
                    {
                        BaseActorAsset = OldBaseActor;
                    }

                    ImGui::Text("Parent: %s", Parent ? Parent->Name : "None");
                }

                glm::vec3 EulerRotation{ glm::degrees(eulerAngles(Transform.Rotation).x),
                                                glm::degrees(eulerAngles(Transform.Rotation).y),
                                                glm::degrees(eulerAngles(Transform.Rotation).z) };

                ImGui::DragFloat3("Translation (x, y, z)", &Transform.Translation.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
                ImGui::DragFloat3("Scale (x, y, z)", &Transform.Scale.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX);
                ImGui::DragFloat3("Rotation", &EulerRotation.x, 0.1f, -360, 360);
                ImGui::Checkbox("Visible", &bVisible);
                ImGui::Spacing();

                Transform.Rotation = glm::quat({ glm::radians(EulerRotation.x), glm::radians(EulerRotation.y), glm::radians(EulerRotation.z) });
            }
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
            NewActorAsset->InternalSaveToResourceFile(NewActorAsset->ResourcePath);
        }
        else
        {
            assert(ResourcePath.GetLength());
            InternalSaveToResourceFile(ResourcePath);
        }
    }

    void IActor::OnAddToWorld(CWorld* InWorld)
    {
        for (u32 i = 0; i < Children.GetLength(); ++i)
        {
            auto* ChildActor = *Children[i];
            ChildActor->OnAddToWorld(InWorld);
        }
        World = InWorld;
    }

    void IActor::OnRemoveFromWorld()
    {
        for (u32 i = 0; i < Children.GetLength(); ++i)
        {
            auto* ChildActor = *Children[i];
            ChildActor->OnRemoveFromWorld();
        }
    }

    void IActor::DrawGizmos(CCamera const* InCamera)
    {
        static ImGuizmo::OPERATION  CurrentGizmoOperation(ImGuizmo::ROTATE);
        static ImGuizmo::MODE       CurrentGizmoMode(ImGuizmo::WORLD);

        if (IsKeyPressed(SDLK_t))
            CurrentGizmoOperation = ImGuizmo::TRANSLATE;
        else if (IsKeyPressed(SDLK_r))
            CurrentGizmoOperation = ImGuizmo::ROTATE;
        else if (IsKeyPressed(SDLK_e))
            CurrentGizmoOperation = ImGuizmo::SCALE;

        if (CurrentGizmoOperation != ImGuizmo::SCALE)
        {
            // if (ImGui::RadioButton("Local", CurrentGizmoMode == ImGuizmo::LOCAL))
            //     CurrentGizmoMode = ImGuizmo::LOCAL;
            // ImGui::SameLine();
            // if (ImGui::RadioButton("World", CurrentGizmoMode == ImGuizmo::WORLD))
            //     CurrentGizmoMode = ImGuizmo::WORLD;
        }
        
        static bool UseSnap;
        if (ImGui::IsKeyPressed(SDLK_y))
            UseSnap = !UseSnap;

        // glm::vec3 Snap;
        // switch (CurrentGizmoOperation)
        // {
        // case ImGuizmo::TRANSLATE:
        //     snap = {1, 0, 0}
        //     ImGui::InputFloat3("Snap", &snap.x);
        //     break;
        // case ImGuizmo::ROTATE:
        //     snap = config.mSnapRotation;
        //     ImGui::InputFloat("Angle Snap", &snap.x);
        //     break;
        // case ImGuizmo::SCALE:
        //     snap = config.mSnapScale;
        //     ImGui::InputFloat("Scale Snap", &snap.x);
        //     break;
        // }

        glm::mat4 ViewMatrix = InCamera->GetViewMatrix();
        glm::mat4 ProjectionMatrix = InCamera->GetProjectionMatrix();
        glm::mat4 ModelMatrix = CalculateModelMatrix();

        if (ImGuizmo::Manipulate(&ViewMatrix[0][0], &ProjectionMatrix[0][0], CurrentGizmoOperation, CurrentGizmoMode, &ModelMatrix[0][0], NULL, NULL))
        {
            glm::vec3 Skew;
            glm::vec4 Perspective;
            glm::vec3 Translation;
            glm::vec3 Scale;
            glm::quat Rotation;
            glm::decompose(ModelMatrix, Scale, Rotation, Translation, Skew, Perspective);

            if (CurrentGizmoOperation == ImGuizmo::TRANSLATE)
            {
                Transform.Translation = Translation;
            }
            else if (CurrentGizmoOperation == ImGuizmo::ROTATE)
            {
                Transform.Rotation = Rotation;
            }
            else
            {
                Transform.Scale = Scale;
            }
        }
    }
} // namespace lucid::scene
