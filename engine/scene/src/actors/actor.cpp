#include "scene/actors/actor.hpp"

#include "scene/camera.hpp"
#include "engine/engine.hpp"

#include "imgui.h"
#include "ImGuizmo.h"
#include "imgui_lucid.h"

#include <sole/sole.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <scene/world.hpp>

#include "platform/input.hpp"
#include "platform/util.hpp"

namespace lucid::scene
{

#if DEVELOPMENT
    void IActor::UIDrawActorDetails()
    {
        if (!BaseActorAsset)
        {
            if (ImGui::Button("Save"))
            {
                SaveAssetToFile();
            }
        }

        // Common actor options
        if (ImGui::CollapsingHeader("Actor"))
        {
            static char RenameBuffer[256];

            // Renaming
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
                ImGui::Text(AssetId == sole::INVALID_UUID ? "Actor name: %s" : "Actor asset name: %s", *Name);
                ImGui::SameLine();
                if (ImGui::Button("Rename"))
                {
                    Name.CopyToBuffer(RenameBuffer);
                    bRenaming = true;
                }

                ImGui::Spacing();
            }

            // Don't allow do modify translation and similar things on actor assets
            if (AssetId == sole::INVALID_UUID)
            {
                PrevBaseActorAsset   = nullptr;
                IActor* OldBaseActor = BaseActorAsset;

                if (BaseActorAsset)
                {
                    // Base asset panel
                    ImGui::Text("Base actor asset:");
                    ImGuiActorAssetPicker("Base actor asset", &BaseActorAsset, GetActorType());

                    if (BaseActorAsset && BaseActorAsset != OldBaseActor && BaseActorAsset->GetActorType() == OldBaseActor->GetActorType())
                    {
                        PrevBaseActorAsset = OldBaseActor;
                        if (!BaseActorAsset->bAssetResourcesLoaded)
                        {
                            BaseActorAsset->LoadAssetResources();
                            BaseActorAsset->bAssetResourcesLoaded = true;
                        }

                        BaseActorAsset->AssetReferences.Add(this);
                        PrevBaseActorAsset->AssetReferences.Remove(this);
                        if (PrevBaseActorAsset->AssetReferences.IsEmpty())
                        {
                            PrevBaseActorAsset->UnloadAssetResources();
                        }
                    }
                    else
                    {
                        BaseActorAsset = OldBaseActor;
                    }

                    ImGui::Spacing();

                    // Parent panel
                    if (bParentable)
                    {
                        static bool bChangingParent = false;
                        ImGui::Text("Parent: %s", Parent ? *Parent->Name : "None");
                        ImGui::SameLine();
                        if (bChangingParent)
                        {
                            if (ImGui::Button("Submit"))
                            {
                                bChangingParent = false;
                            }
                        }
                        else
                        {
                            if (ImGui::Button("Change parent"))
                            {
                                bChangingParent = true;
                            }
                        }

                        if (bChangingParent)
                        {
                            if (ImGui::BeginListBox("### possible parrents"))
                            {
                                if (ImGui::Selectable("None", Parent == nullptr))
                                {
                                    if (Parent)
                                    {
                                        Parent->RemoveChild(this);
                                    }
                                    Parent = nullptr;
                                }

                                for (u32 i = 0; i < World->GetActorsMap().GetLength(); ++i)
                                {
                                    auto* Actor = World->GetActorsMap().GetByIndex(i);
                                    if (Actor == this)
                                    {
                                        continue;
                                    }
                                    ImGui::PushID((void*)(intptr_t)Actor->ActorId);
                                    if (ImGui::Selectable(
                                          "##actor", Parent == Actor, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_SpanAllColumns))
                                    {
                                        if (Parent)
                                        {
                                            Parent->RemoveChild(this);
                                        }
                                        Parent = Actor;
                                        Parent->AddChild(this);
                                    }
                                    ImGui::PopID();
                                    ImGui::SameLine();
                                    ImGui::Text(*Actor->Name);
                                    if (Parent == Actor)
                                    {
                                        ImGui::SetItemDefaultFocus();
                                    }
                                }
                                ImGui::EndListBox();
                            }
                        }
                    }
                }

                // Transforms panel
                if (bMovable)
                {
                    ImGui::Spacing();
                    glm::vec3 EulerRotation{ glm::degrees(eulerAngles(Transform.Rotation).x),
                                             glm::degrees(eulerAngles(Transform.Rotation).y),
                                             glm::degrees(eulerAngles(Transform.Rotation).z) };

                    bTranslationUpdated = false;
                    bScaleUpdated       = false;
                    bRotationUpdated    = false;

                    if (ImGui::DragFloat3("Translation (x, y, z)", &Transform.Translation.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX))
                    {
                        bTranslationUpdated = true;
                    }
                    if (ImGui::DragFloat3("Scale (x, y, z)", &Transform.Scale.x, 0.005f, -FLT_HALF_MAX, FLT_HALF_MAX))
                    {
                        bScaleUpdated = true;
                    }
                    if (ImGui::DragFloat3("Rotation", &EulerRotation.x, 0.1f, -360, 360))
                    {
                        bRotationUpdated = true;
                    }
                    ImGui::Checkbox("Visible", &bVisible);
                    ImGui::Spacing();

                    Transform.Rotation = glm::quat({ glm::radians(EulerRotation.x), glm::radians(EulerRotation.y), glm::radians(EulerRotation.z) });
                }
            }
        }
    }

    IActor* IActor::UIDrawHierarchy()
    {
        // Draw a tree when actor has children
        if (Children.Head.Element)
        {
            IActor* ClickedActor = nullptr;
            if (ImGui::TreeNode((void*)(intptr_t)ActorId, *Name))
            {
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    ClickedActor = this;
                }
                auto ChildNode = &Children.Head;
                while (ChildNode && ChildNode->Element)
                {
                    if (auto* ClickedChild = ChildNode->Element->UIDrawHierarchy())
                    {
                        ClickedActor = ClickedChild;
                    }
                    ChildNode = ChildNode->Next;
                }
                ImGui::TreePop();
            }
            return ClickedActor;
        }

        // Otherwise just draw text
        ImGui::PushID((void*)(intptr_t)ActorId);
        if (ImGui::Selectable(*Name))
        {
            ImGui::PopID();
            return this;
        }
        ImGui::PopID();
        return nullptr;
    }

    void IActor::DrawGizmos(CCamera const* InCamera)
    {
        if (!bMovable)
        {
            return;
        }
        static ImGuizmo::OPERATION CurrentGizmoOperation(ImGuizmo::ROTATE);
        static ImGuizmo::MODE      CurrentGizmoMode(ImGuizmo::WORLD);

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

        glm::mat4 ViewMatrix       = InCamera->GetViewMatrix();
        glm::mat4 ProjectionMatrix = InCamera->GetProjectionMatrix();
        glm::mat4 ModelMatrix      = CalculateModelMatrix();

        if (ImGuizmo::Manipulate(&ViewMatrix[0][0], &ProjectionMatrix[0][0], CurrentGizmoOperation, CurrentGizmoMode, &ModelMatrix[0][0], NULL, NULL))
        {
            glm::vec3 Skew;
            glm::vec4 Perspective;
            glm::vec3 Translation;
            glm::vec3 Scale;
            glm::quat Rotation;
            glm::decompose(ModelMatrix, Scale, Rotation, Translation, Skew, Perspective);

            bTranslationUpdated = false;
            bScaleUpdated       = false;
            bRotationUpdated    = false;

            if (CurrentGizmoOperation == ImGuizmo::TRANSLATE)
            {
                Transform.Translation = Translation;
                bTranslationUpdated   = true;
            }
            else if (CurrentGizmoOperation == ImGuizmo::ROTATE)
            {
                Transform.Rotation = Rotation;
                bRotationUpdated   = true;
            }
            else
            {
                Transform.Scale = Scale;
                bScaleUpdated   = true;
            }
        }
    }

#endif

    void IActor::SaveAssetToFile()
    {
        if (BaseActorAsset)
        {
            auto* NewActorAsset         = CreateAssetFromActor(Name.GetCopy());
            NewActorAsset->AssetId   = sole::uuid4();
            NewActorAsset->AssetPath = SPrintf("assets/actors/%s.asset", *Name);

            FActorDatabaseEntry NewEntry;
            NewEntry.ActorId   = NewActorAsset->AssetId;
            NewEntry.ActorPath = NewActorAsset->AssetPath;

            GEngine.AddActorAsset(NewActorAsset);

            BaseActorAsset = NewActorAsset;
            NewActorAsset->InternalSaveAssetToFile(NewActorAsset->AssetPath);
        }
        else
        {
            assert(AssetPath.GetLength());
            InternalSaveAssetToFile(AssetPath);
        }
    }

    void IActor::OnAddToWorld(CWorld* InWorld)
    {
        auto ChildNode = &Children.Head;
        while (ChildNode && ChildNode->Element)
        {
            ChildNode->Element->OnAddToWorld(InWorld);
            ChildNode = ChildNode->Next;
        }
        World = InWorld;
        if (BaseActorAsset)
        {
            BaseActorAsset->AddAssetReference(this);
        }
    }

    void IActor::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        auto ChildNode = &Children.Head;
        while (ChildNode && ChildNode->Element)
        {
            ChildNode->Element->OnRemoveFromWorld(InbHardRemove);
            if (InbHardRemove)
            {
                delete ChildNode->Element;
            }
            ChildNode = ChildNode->Next;
        }
        if (InbHardRemove)
        {
            Children.Free();
        }
        if (BaseActorAsset)
        {
            BaseActorAsset->RemoveAssetReference(this);
        }
    }

    void IActor::CleanupAfterRemove()
    {
        auto ChildNode = &Children.Head;
        while (ChildNode && ChildNode->Element)
        {
            delete ChildNode->Element;
        }
        Children.Free();
    }

    void IActor::AddAssetReference(IActor* InChildReference)
    {
        InChildReference->BaseActorAsset = this;
        if (AssetReferences.IsEmpty())
        {
            LoadAssetResources();
        }
        AssetReferences.Add(InChildReference);
    }

    void IActor::RemoveAssetReference(IActor* InChildReference)
    {
        AssetReferences.Remove(InChildReference);
        if (AssetReferences.IsEmpty())
        {
            UnloadAssetResources();
        }
    }

} // namespace lucid::scene
