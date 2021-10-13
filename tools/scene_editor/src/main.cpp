#include "engine/engine.hpp"

#include "devices/gpu/gpu.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/viewport.hpp"
#include "devices/gpu/texture_enums.hpp"

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "platform/util.hpp"
#include "platform/platform.hpp"

#include "scene/camera.hpp"
#include "scene/blinn_phong_material.hpp"
#include "scene/actors/static_mesh.hpp"
#include "scene/world.hpp"
#include "scene/flat_material.hpp"
#include "scene/material.hpp"
#include "scene/actors/actor_enums.hpp"
#include "scene/renderer.hpp"
#include "scene/actors/actor.hpp"
#include "scene/actors/lights.hpp"
#include "scene/actors/terrain.hpp"

#include "resources/texture_resource.hpp"
#include "resources/mesh_resource.hpp"

#include "glm/gtc/quaternion.hpp"

#include "schemas/json.hpp"

#include "sole/sole.hpp"

#include "lucid_editor/editor.hpp"
#include "lucid_editor/imgui_lucid.h"

#include <algorithm>
#include <stdio.h>
#include <glm/gtx/matrix_decompose.hpp>

#include "ImGuizmo.h"
#include "scene/pbr_material.hpp"
#include "scene/textured_pbr_material.hpp"

using namespace lucid;

bool EqualIgnoreCase(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == tolower(b); });
}

bool InitializeSceneEditor();

void HandleCameraMovement(const float& DeltaTime);
void DoActorPicking();
void DrawActiveActorGizmos();
void HandleInput();

void UIOpenPopup();
void UISetupDockspace();
void UIDrawSceneWindow();
void UIDrawResourceBrowserWindow();
void UIDrawSceneHierarchyWindow();
void UIDrawActorDetailsWindow();
void UIDrawFileDialog();
void UIDrawMeshImporter();
void UIDrawTextureImporter();
void UIDrawMeshContextMenu();
void UIDrawTextureContextMenu();
void UIDrawMaterialContextMenu();
void UIDrawMaterialCreationMenu();
void UIDrawActorAssetContextMenu();
void UIDrawActorResourceCreationMenu();
void UIDrawCommonActorsWindow();
void UIDrawHelpWindow();
void UIDrawStatsWindow();
void UIDrawSettingsWindows();

void UIDrawDraggableImage(const char*    InDragDropId,
                          gpu::CTexture* InTexture,
                          const char*    InLabel,
                          const ImVec2&  InSize,
                          const char*    InDragDataType,
                          const void*    InData,
                          const u32&     InDataSize);

void ImportTexture(const std::filesystem::path& SelectedFilePath);
void ImportMesh(const std::filesystem::path& SelectedFilePath);
void LoadWorld(const std::filesystem::path& SelectedFilePath);

int main(int argc, char** argv)
{
    if (!InitializeSceneEditor())
    {
        return -1;
    }

    // Load textures and meshes used in the demo scene
    gpu::SetClearColor(BlackColor);

    real now  = platform::GetCurrentTimeSeconds();
    real last = 0;
    real dt   = 0;

    scene::FRenderView RenderView;
    RenderView.Camera   = GSceneEditorState.CurrentCamera; // @TODO is this needed???
    RenderView.Viewport = { 0, 0, 1920, 1080 }; // @TODO Engine level variable

    while (GSceneEditorState.bIsRunning)
    {
        GSceneEditorState.Window->Prepare();

        if (GSceneEditorState.PendingDeleteWorld)
        {
            GEngine.GetRenderer()->ResetState();
            GSceneEditorState.PendingDeleteWorld->Unload();
            delete GSceneEditorState.PendingDeleteWorld;
            GSceneEditorState.PendingDeleteWorld = nullptr;
        }

        GEngine.BeginFrame();

        platform::Update();
        last = now;
        now  = platform::GetCurrentTimeSeconds();
        dt += now - last;

        ReadEvents(GSceneEditorState.Window);
        HandleInput();

        if (dt > platform::SimulationStep)
        {
            dt -= platform::SimulationStep;

            HandleCameraMovement(platform::SimulationStep);

            GSceneEditorState.CurrentCamera->Tick(platform::SimulationStep);
            GSceneEditorState.SecondsSinceLastVideoMemorySnapshot += platform::SimulationStep;

            if (GSceneEditorState.SecondsSinceLastVideoMemorySnapshot > 1)
            {
                GSceneEditorState.VideoMemoryUsage[GSceneEditorState.VideoMemoryUsageIndex++ % GSceneEditorState.NumVideoMemoryUsageSamples] =
                  float(gpu::GGPUStatus.TotalAvailableVideoMemoryKB - gpu::GGPUStatus.CurrentAvailableVideoMemoryKB) / 1024.f;
                GSceneEditorState.SecondsSinceLastVideoMemorySnapshot = 0;
            }

            if (GSceneEditorState.World)
            {
                GSceneEditorState.World->Tick(platform::SimulationStep);
            }
        }

        if (GSceneEditorState.CurrentlySelectedActor)
        {
            GSceneEditorState.CurrentlySelectedActor->OnSelectedPreFrameRender();
        }

        // Render the scene to off-screen framebuffer
        if (GSceneEditorState.World)
        {
            GEngine.GetRenderer()->Render(GSceneEditorState.World->MakeRenderScene(GSceneEditorState.CurrentCamera), &RenderView);
        }

        GSceneEditorState.NumDrawCalls[GSceneEditorState.NumDrawCallsIndex++ % GSceneEditorState.NumDrawCallSamples] = scene::GRenderStats.NumDrawCalls;
        GSceneEditorState.FrameTimes[GSceneEditorState.FrameTimesIndex++ % (GSceneEditorState.NumFrameTimesSamples)] = scene::GRenderStats.FrameTimeMiliseconds;

        DoActorPicking();

        GSceneEditorState.Window->ImgUiStartNewFrame();
        {
            UISetupDockspace();
            UIDrawSceneWindow();
            UIDrawResourceBrowserWindow();
            UIDrawSceneHierarchyWindow();
            UIDrawActorDetailsWindow();
            UIDrawCommonActorsWindow();
            UIDrawFileDialog();
            UIDrawStatsWindow();
            UIDrawSettingsWindows();
            ImGui::ShowDemoWindow();
        }

        GSceneEditorState.Window->Clear();
        GSceneEditorState.Window->ImgUiDrawFrame();
        GSceneEditorState.Window->Swap();

        // Allow ImGui viewports to update
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        GEngine.EndFrame();
    }

    GEngine.Shutdown();
    return 0;
}

bool InitializeSceneEditor()
{
    // Create the window for the editor

    FEngineConfig EngineConfig;
    EngineConfig.bHotReloadShaders = true;

    if (GEngine.InitEngine(EngineConfig) != EEngineInitError::NONE)
    {
        return false;
    }

    platform::FWindowDefiniton EditorWindow;
    EditorWindow.title           = "Lucid Editor";
    EditorWindow.X               = 100;
    EditorWindow.Y               = 100;
    EditorWindow.Width           = GSceneEditorState.EditorWindowWidth;
    EditorWindow.Height          = GSceneEditorState.EditorWindowHeight;
    EditorWindow.sRGBFramebuffer = true;
    EditorWindow.bHidden         = false;

    GSceneEditorState.Window = platform::CreateNewWindow(EditorWindow);
    GSceneEditorState.Window->Prepare();

    GEngine.LoadResources();

    // Setup ImGui
    IMGUI_CHECKVERSION();

    GSceneEditorState.ImGuiContext = ImGui::CreateContext();

    // Configure ImGui
    ImGuiIO& ImGuiIO = ImGui::GetIO();
    ImGuiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGuiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGuiIO.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup img ui
    GSceneEditorState.Window->ImgUiSetup();

    // Setup the initial layout
    GSceneEditorState.Window->ImgUiStartNewFrame();
    {
        GSceneEditorState.MainDockId = ImGui::GetID(MAIN_DOCKSPACE);
        ImGui::Begin(DOCKSPACE_WINDOW, nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking);
        {
            // Remove old layout
            ImGui::DockBuilderRemoveNode(GSceneEditorState.MainDockId);

            // Create the dockspace
            ImGui::DockSpace(GSceneEditorState.MainDockId, ImVec2(0.0f, 0.0f));
            ImGui::DockBuilderSetNodeSize(GSceneEditorState.MainDockId,
                                          { (float)GSceneEditorState.EditorWindowWidth, (float)GSceneEditorState.EditorWindowHeight });

            // Split the dockspace
            ImGuiID ResourceBrowserWindowDockId;
            ImGuiID SceneHierarchyDockId;
            ImGuiID ActorDetailsDockId;
            ImGuiID SceneWindowDockId;
            ImGuiID CommonActorsDockId;
            SceneWindowDockId    = ImGui::DockBuilderSplitNode(GSceneEditorState.MainDockId, ImGuiDir_Left, 0.75f, nullptr, &SceneHierarchyDockId);
            SceneWindowDockId    = ImGui::DockBuilderSplitNode(SceneWindowDockId, ImGuiDir_Up, 0.7f, nullptr, &ResourceBrowserWindowDockId);
            SceneWindowDockId    = ImGui::DockBuilderSplitNode(SceneWindowDockId, ImGuiDir_Right, 0.9f, nullptr, &CommonActorsDockId);
            SceneHierarchyDockId = ImGui::DockBuilderSplitNode(SceneHierarchyDockId, ImGuiDir_Up, 0.5f, nullptr, &ActorDetailsDockId);

            // Attach windows to dockspaces
            ImGui::DockBuilderDockWindow(SCENE_VIEWPORT, SceneWindowDockId);
            ImGui::DockBuilderDockWindow(RESOURCES_BROWSER, ResourceBrowserWindowDockId);
            ImGui::DockBuilderDockWindow(ASSETS_BROWSER, ResourceBrowserWindowDockId);
            ImGui::DockBuilderDockWindow(SCENE_HIERARCHY, SceneHierarchyDockId);
            ImGui::DockBuilderDockWindow(ACTOR_DETAILS, ActorDetailsDockId);
            ImGui::DockBuilderDockWindow(COMMON_ACTORS, CommonActorsDockId);

            // Submit the layout
            ImGui::DockBuilderFinish(GSceneEditorState.MainDockId);

            ImGui::Begin(SCENE_VIEWPORT);
            {
                GSceneEditorState.ImSceneWindow = ImGui::GetCurrentWindow();
            };
            ImGui::End();
        }
        ImGui::End();
    }

    // Configure the camera
    GSceneEditorState.PerspectiveCamera.SetAspectRatio(GSceneEditorState.ImSceneWindow->Size.x / GSceneEditorState.ImSceneWindow->Size.y);
    GSceneEditorState.PerspectiveCamera.SetPosition({ 0, 0, 0 });
    GSceneEditorState.PerspectiveCamera.SetYaw(-90.f);
    GSceneEditorState.PerspectiveCamera.UpdateCameraVectors();

    GSceneEditorState.CurrentCamera = &GSceneEditorState.PerspectiveCamera;

    GSceneEditorState.Window->ImgUiDrawFrame();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    return true;
}

void HandleCameraMovement(const float& DeltaTime)
{
    // Check if we're inside the scene window
    const ImVec2 MousePositionAbs = ImGui::GetMousePos();
    const ImVec2 SceneWindowPos   = GSceneEditorState.SceneWindowPos;
    if (MousePositionAbs.x < SceneWindowPos.x || MousePositionAbs.x > (SceneWindowPos.x + GSceneEditorState.SceneWindowWidth) ||
        MousePositionAbs.y < SceneWindowPos.y || MousePositionAbs.y > (SceneWindowPos.y + GSceneEditorState.SceneWindowHeight))
    {
        return;
    }

    if (GSceneEditorState.bDisableCameraMovement)
    {
        return;
    }

    const FMousePosition MousePos = GetMousePostion();
    if (IsKeyPressed(SDLK_w))
    {
        GSceneEditorState.CurrentCamera->MoveForward(platform::SimulationStep);
    }

    if (IsKeyPressed(SDLK_s))
    {
        GSceneEditorState.CurrentCamera->MoveBackward(platform::SimulationStep);
    }

    if (IsKeyPressed(SDLK_a))
    {
        GSceneEditorState.CurrentCamera->MoveLeft(platform::SimulationStep);
    }

    if (IsKeyPressed(SDLK_d))
    {
        GSceneEditorState.CurrentCamera->MoveRight(platform::SimulationStep);
    }

    if (IsMouseButtonPressed(RIGHT))
    {
        if (!GSceneEditorState.bBlockActorPicking)
        {
            GSceneEditorState.CurrentlySelectedActor = nullptr;
        }
        GSceneEditorState.CurrentCamera->AddRotation(-MousePos.DeltaX * .9f, MousePos.DeltaY * .9f);
    }
}

void DoActorPicking()
{
    if (GSceneEditorState.bBlockActorPicking)
    {
        return;
    }

    if (scene::IActor* ClickedActor = SceneWindow_GetActorUnderCursor())
    {
        if (GSceneEditorState.CurrentlySelectedActor == nullptr)
        {
            // Remember the actor that we hit and how far from the camera it was on the z axis
            GSceneEditorState.CurrentlySelectedActor = ClickedActor;
        }
    }
}

void HandleInput()
{
    if (GSceneEditorState.Window->IsRequestedClose())
    {
        GSceneEditorState.bIsRunning = false;
    }

    if (IsKeyPressed(SDLK_c) && IsKeyPressed(SDLK_LCTRL))
    {
        if (GSceneEditorState.CurrentlySelectedActor)
        {
            GSceneEditorState.ClipboardActor = GSceneEditorState.CurrentlySelectedActor;
        }
    }

    if (IsKeyPressed(SDLK_v) && IsKeyPressed(SDLK_LCTRL))
    {
        if (GSceneEditorState.ClipboardActor && (platform::GetCurrentTimeSeconds() - GSceneEditorState.LastActorSpawnTime) > 0.5)
        {
            GSceneEditorState.LastActorSpawnTime = platform::GetCurrentTimeSeconds();
            GSceneEditorState.ClipboardActor->CreateActorCopy();
        }
    }

    if (IsKeyPressed(SDLK_DELETE) && GSceneEditorState.CurrentlySelectedActor)
    {
        if (GSceneEditorState.LastDeletedActor)
        {
            GSceneEditorState.LastDeletedActor->CleanupAfterRemove();
            delete GSceneEditorState.LastDeletedActor;
        }
        GSceneEditorState.World->RemoveActorById(GSceneEditorState.CurrentlySelectedActor->ActorId, false);
        GSceneEditorState.LastDeletedActor       = GSceneEditorState.CurrentlySelectedActor;
        GSceneEditorState.CurrentlySelectedActor = nullptr;
    }

    if (IsKeyPressed(SDLK_z) && IsKeyPressed(SDLK_LCTRL))
    {
        if (GSceneEditorState.LastDeletedActor)
        {
            GSceneEditorState.LastDeletedActor->OnAddToWorld(GSceneEditorState.World);
            GSceneEditorState.LastDeletedActor = nullptr;
        }
    }

    if (IsKeyPressed(SDLK_c) && GSceneEditorState.CurrentlySelectedActor)
    {
        const scene::IActor* ActorToFocusOn   = GSceneEditorState.CurrentlySelectedActor;
        const glm::vec3&     ToCameraPosition = (ActorToFocusOn->GetAABB().FrontUpperLeftCorner - ActorToFocusOn->GetTransform().Translation);
        const glm::vec3      CameraPosition   = ActorToFocusOn->GetTransform().Translation + (ToCameraPosition * 2.f);
        const glm::vec3&     FocusPoint       = ActorToFocusOn->GetTransform().Translation;
        glm::vec3            CameraDirection  = normalize(FocusPoint - CameraPosition);

        if (glm::isnan(CameraDirection.x) || glm::isnan(CameraDirection.y) || glm::isnan(CameraDirection.z))
        {
            CameraDirection = ActorToFocusOn->GetActorForwardVector();
        }

        GSceneEditorState.CurrentCamera->MoveToOverTime(CameraPosition, CameraDirection, 0.5f);
    }
}

void UIOpenPopup(const char* InTitle)
{
    if (!ImGui::IsPopupOpen(POPUP_WINDOW))
    {
        ImGui::OpenPopup(POPUP_WINDOW);
    }

    ImGui::SetNextWindowPos({ GSceneEditorState.Window->GetPosition().x + GSceneEditorState.Window->GetWidth() * 0.5f,
                              GSceneEditorState.Window->GetPosition().y + GSceneEditorState.Window->GetHeight() * 0.5f },
                            ImGuiCond_Always,
                            { 0.5f, 0.5f });
    ImGui::SetNextWindowSize({ 0, 0 });
    ImGui::BeginPopupModal(POPUP_WINDOW, nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::Text(InTitle);
}

void UISetupDockspace()
{
    // Create the dockspace window which will occupy the whole editor window
    ImGuiWindowFlags DockspaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    DockspaceWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    DockspaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin(DOCKSPACE_WINDOW, nullptr, DockspaceWindowFlags);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // World operations
            if (ImGui::MenuItem("Load world"))
            {
                if (GSceneEditorState.World)
                {
                    if (GSceneEditorState.WorldFilePath.GetLength())
                    {
                        GSceneEditorState.ClipboardActor         = nullptr;
                        GSceneEditorState.CurrentlySelectedActor = nullptr;
                        GSceneEditorState.LastDeletedActor       = nullptr;
                        GSceneEditorState.World->SaveToJSONFile(GSceneEditorState.WorldFilePath);
                        GSceneEditorState.PendingDeleteWorld = GSceneEditorState.World;
                        GSceneEditorState.World              = nullptr;
                    }
                }
                GSceneEditorState.FileDialog.SetTitle("Select a world file");
                GSceneEditorState.FileDialog.SetTypeFilters({ ".asset" });
                GSceneEditorState.OnFileSelected  = &LoadWorld;
                GSceneEditorState.bShowFileDialog = true;
                GSceneEditorState.FileDialog.ClearSelected();
                GSceneEditorState.FileDialog.Open();
            }

            if (GSceneEditorState.World)
            {
                if (ImGui::MenuItem("Save world"))
                {
                    if (GSceneEditorState.WorldFilePath.GetLength())
                    {
                        GSceneEditorState.World->SaveToJSONFile(*GSceneEditorState.WorldFilePath);
                    }
                    else
                    {
                        Zero(GSceneEditorState.AssetNameBuffer, 256);
                        GSceneEditorState.bSavingWorldToFile     = true;
                        GSceneEditorState.bCreatingNewWorld      = false;
                        GSceneEditorState.bDisableCameraMovement = true;
                    }
                }
            }

            if (ImGui::MenuItem("New world"))
            {
                if (GSceneEditorState.World)
                {
                    if (GSceneEditorState.WorldFilePath.GetLength())
                    {
                        GSceneEditorState.World->SaveToJSONFile(*GSceneEditorState.WorldFilePath);
                        GSceneEditorState.PendingDeleteWorld     = GSceneEditorState.World;
                        GSceneEditorState.World                  = new scene::CWorld;
                        GSceneEditorState.ClipboardActor         = nullptr;
                        GSceneEditorState.CurrentlySelectedActor = nullptr;
                        GSceneEditorState.LastDeletedActor       = nullptr;
                        GSceneEditorState.WorldFilePath.Free();
                        GSceneEditorState.bSavingWorldToFile = false;
                    }
                    else
                    {
                        Zero(GSceneEditorState.AssetNameBuffer, 256);
                        GSceneEditorState.bSavingWorldToFile = true;
                    }
                    GSceneEditorState.bCreatingNewWorld = true;
                }
                else
                {
                    GSceneEditorState.bSavingWorldToFile     = false;
                    GSceneEditorState.bCreatingNewWorld      = true;
                    GSceneEditorState.bDisableCameraMovement = false;

                    GSceneEditorState.World                  = new scene::CWorld;
                    GSceneEditorState.ClipboardActor         = nullptr;
                    GSceneEditorState.CurrentlySelectedActor = nullptr;
                    GSceneEditorState.LastDeletedActor       = nullptr;
                }
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Renderer") && !GSceneEditorState.bShowingControlsWindow)
            {
                GSceneEditorState.bShowingRendererSettinsWindow = true;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Controls") && !GSceneEditorState.bShowingControlsWindow)
            {
                GSceneEditorState.bShowingControlsWindow = true;
            }

            if (ImGui::MenuItem("Stats") && !GSceneEditorState.bShowingStatsWindow)
            {
                GSceneEditorState.bShowingStatsWindow = true;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::PopStyleVar(2);
    ImGui::DockSpace(GSceneEditorState.MainDockId, ImVec2(0.0f, 0.0f));
    ImGui::End();

    if (GSceneEditorState.bSavingWorldToFile)
    {
        UIOpenPopup("World");
        {
            ImGui::InputText("World name (max 255)", GSceneEditorState.AssetNameBuffer, 255);
            if (ImGui::Button("Submit"))
            {
                if (strnlen_s(GSceneEditorState.AssetNameBuffer, 256) > 0)
                {
                    GSceneEditorState.WorldFilePath = SPrintf("assets/worlds/%s.asset", GSceneEditorState.AssetNameBuffer);
                    GSceneEditorState.World->SaveToJSONFile(*GSceneEditorState.WorldFilePath);
                    if (GSceneEditorState.bCreatingNewWorld)
                    {
                        if (GSceneEditorState.World)
                        {
                            GSceneEditorState.PendingDeleteWorld = GSceneEditorState.World;
                        }
                        GSceneEditorState.World                  = new scene::CWorld;
                        GSceneEditorState.ClipboardActor         = nullptr;
                        GSceneEditorState.CurrentlySelectedActor = nullptr;
                        GSceneEditorState.LastDeletedActor       = nullptr;
                        GSceneEditorState.WorldFilePath.Free();
                    }

                    GSceneEditorState.bCreatingNewWorld      = false;
                    GSceneEditorState.bSavingWorldToFile     = false;
                    GSceneEditorState.bDisableCameraMovement = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                GSceneEditorState.bSavingWorldToFile     = false;
                GSceneEditorState.bDisableCameraMovement = false;
                GSceneEditorState.bCreatingNewWorld      = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void UIDrawSceneWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin(SCENE_VIEWPORT, nullptr, WindowFlags);
    {
        // Update scene editor state so it can be used later in the frame
        const ImVec2 SceneWindowPos   = ImGui::GetWindowPos();
        const ImVec2 WindowContentMin = ImGui::GetWindowContentRegionMin();
        const ImVec2 WindowContentMax = ImGui::GetWindowContentRegionMax();

        GSceneEditorState.SceneWindowWidth  = WindowContentMax.x - WindowContentMin.x;
        GSceneEditorState.SceneWindowHeight = WindowContentMax.y - WindowContentMin.y;

        // This gets the position of the content area, excluding title bars and etc.
        GSceneEditorState.SceneWindowPos = WindowContentMin;
        GSceneEditorState.SceneWindowPos.x += SceneWindowPos.x;
        GSceneEditorState.SceneWindowPos.y += SceneWindowPos.y;

        // Draw the rendered scene into an image
        GEngine.GetRenderer()->GetSelectedDebugTexture()->ImGuiDrawToImage({ GSceneEditorState.SceneWindowWidth, GSceneEditorState.SceneWindowHeight });

        // Handle actor asset drag and drop into the viewport
        if (GSceneEditorState.World)
        {
            if (ImGui::BeginDragDropTargetCustom(GSceneEditorState.ImSceneWindow->Rect(), GSceneEditorState.ImSceneWindow->ID))
            {
                if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload(ACTOR_ASSET_DRAG_TYPE))
                {
                    IM_ASSERT(Payload->DataSize == sizeof(sole::uuid));
                    const sole::uuid ActorAssetId = *(sole::uuid*)Payload->Data;
                    if (scene::IActor* ActorAsset = GEngine.GetActorsResources().Get(ActorAssetId))
                    {
                        const glm::vec2 MouseNDCPos              = GetMouseNDCPos();
                        const glm::vec3 SpawnedActorPos          = GSceneEditorState.CurrentCamera->GetMouseRayInWorldSpace(MouseNDCPos, 5);
                        auto*           SpawnedActor             = ActorAsset->CreateActorInstanceFromAsset(GSceneEditorState.World, SpawnedActorPos);
                        GSceneEditorState.CurrentlySelectedActor = SpawnedActor;
                    }
                }
                // Handle common actor drag and drop into the viewport
                else if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload(COMMON_ACTOR_DRAG_TYPE))
                {
                    IM_ASSERT(Payload->DataSize == sizeof(ECommonActorType));

                    const ECommonActorType CommonActorType = *(ECommonActorType*)Payload->Data;
                    scene::IActor*         SpawnedActor    = nullptr;
                    switch (CommonActorType)
                    {
                    case ECommonActorType::DIRECTIONAL_LIGHT:
                    {
                        auto* LightActor = new scene::CDirectionalLight{ CopyToString("DirectionalLight"), nullptr, GSceneEditorState.World };
                        GSceneEditorState.World->AddDirectionalLight(LightActor);
                        SpawnedActor = LightActor;
                        break;
                    }
                    case ECommonActorType::SPOT_LIGHT:
                    {
                        auto* LightActor = new scene::CSpotLight{ CopyToString("SpotLight"), nullptr, GSceneEditorState.World };
                        GSceneEditorState.World->AddSpotLight(LightActor);
                        SpawnedActor = LightActor;
                        break;
                    }
                    case ECommonActorType::POINT_LIGHT:
                    {
                        auto* LightActor = new scene::CPointLight{ CopyToString("PointLight"), nullptr, GSceneEditorState.World };
                        GSceneEditorState.World->AddPointLight(LightActor);
                        SpawnedActor = LightActor;
                        break;
                    }
                    }

                    if (SpawnedActor)
                    {
                        SpawnedActor->SetTranslation(GSceneEditorState.CurrentCamera->GetMouseRayInWorldSpace(GetMouseNDCPos(), 5));
                    }
                }

                ImGui::EndDragDropTarget();
            }
        }
    }
    ImGui::PopStyleVar(1);
    ImGui::End();
}

void UIDrawResourceBrowserWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;

    float  ResourceItemWidth = 0;
    ImVec2 ResourceItemSize;
    int    CurrentRowItemsCount = 0;

    ImGui::Begin(RESOURCES_BROWSER, nullptr, WindowFlags);
    {
        ResourceItemWidth = (ImGui::GetContentRegionAvailWidth() / GSceneEditorState.ResourceItemsPerRow) - 8;
        ResourceItemSize  = { ResourceItemWidth, ResourceItemWidth };

        // Menu bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Resource import
                if (ImGui::BeginMenu("Import resource"))
                {
                    if (ImGui::MenuItem("Mesh"))
                    {
                        GSceneEditorState.FileDialog.SetTitle("Select a mesh file");
                        GSceneEditorState.FileDialog.SetTypeFilters({ ".obj" });
                        GSceneEditorState.bShowFileDialog = true;
                        GSceneEditorState.OnFileSelected  = &ImportMesh;
                        GSceneEditorState.FileDialog.ClearSelected();
                        GSceneEditorState.FileDialog.Open();
                    }

                    if (ImGui::MenuItem("Texture"))
                    {
                        GSceneEditorState.FileDialog.SetTitle("Select a texture file");
                        GSceneEditorState.FileDialog.SetTypeFilters({ ".png", ".jpg", ".jpeg", ".tga" });
                        GSceneEditorState.OnFileSelected  = &ImportTexture;
                        GSceneEditorState.bShowFileDialog = true;
                        GSceneEditorState.FileDialog.ClearSelected();
                        GSceneEditorState.FileDialog.Open();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Resources browser
        ImGui::BeginChild("resources scroll");
        {
            // Draw texture resource items
            CurrentRowItemsCount = 0;
            ImGui::Text("Textures:");
            for (int i = 0; i < GEngine.GetTexturesHolder().Length(); ++i)
            {
                // Calculate X position
                resources::CTextureResource* TextureResource = GEngine.GetTexturesHolder().GetByIndex(i);
                if (CurrentRowItemsCount > 0 && (CurrentRowItemsCount % GSceneEditorState.ResourceItemsPerRow) == 0)
                {
                    CurrentRowItemsCount = 0;
                }
                else if (CurrentRowItemsCount > 0)
                {
                    ImGui::SameLine(ResourceItemWidth * CurrentRowItemsCount + (4 * CurrentRowItemsCount), 2);
                }

                // Draw texture thumb and it's name
                if (TextureResource)
                {
                ImGui:
                    ImGui::SetNextItemWidth(ResourceItemWidth);
                    ImGui::BeginGroup();
                    if (TextureResource->GetThumbnail())
                    {
                        TextureResource->GetThumbnail()->ImGuiImageButton(ResourceItemSize);
                    }
                    ImGui::Button(*TextureResource->GetName(), { ResourceItemWidth, 0 });
                    ImGui::EndGroup();
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        GSceneEditorState.ClickedTextureResource = TextureResource;
                    }
                    ++CurrentRowItemsCount;
                }
            }

            ImGui::Spacing();

            // Draw meshes resource items
            CurrentRowItemsCount = 0;
            ImGui::Text("Meshes:");

            for (int i = 0; i < GEngine.GetMeshesHolder().Length(); ++i)
            {
                // Calculate X position
                resources::CMeshResource* MeshResource = GEngine.GetMeshesHolder().GetByIndex(i);
                if (CurrentRowItemsCount > 0 && (CurrentRowItemsCount % GSceneEditorState.ResourceItemsPerRow) == 0)
                {
                    CurrentRowItemsCount = 0;
                }
                else if (CurrentRowItemsCount > 0)
                {
                    ImGui::SameLine(ResourceItemWidth * CurrentRowItemsCount + (4 * CurrentRowItemsCount), 2);
                }

                // Draw mesh thumb and it's name
                if (MeshResource)
                {
                    ImGui::BeginGroup();
                    // if (MeshResource->GetThumbnail())
                    // {
                    //     MeshResource->GetThumbnail()->ImGuiImageButton(ResourceItemSize);
                    // }

                    ImGui::Button(*MeshResource->GetName(), { ResourceItemWidth, MeshResource->GetThumbnail() ? 0 : ResourceItemWidth });
                    ImGui::EndGroup();
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        GSceneEditorState.ClickedMeshResource = MeshResource;
                    }
                    ++CurrentRowItemsCount;
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (GSceneEditorState.bIsImportingMesh)
    {
        UIDrawMeshImporter();
    }
    else if (GSceneEditorState.bIsImportingTexture)
    {
        UIDrawTextureImporter();
    }
    else if (GSceneEditorState.ClickedMeshResource)
    {
        UIDrawMeshContextMenu();
    }
    else if (GSceneEditorState.ClickedTextureResource)
    {
        UIDrawTextureContextMenu();
    }
    else if (GSceneEditorState.ClickedMaterialAsset)
    {
        UIDrawMaterialContextMenu();
    }
    else if (GSceneEditorState.ClickedActorAsset)
    {
        UIDrawActorAssetContextMenu();
    }
    else if (GSceneEditorState.TypeOfMaterialToCreate != scene::EMaterialType::NONE)
    {
        UIDrawMaterialCreationMenu();
    }
    else if (GSceneEditorState.TypeOfActorToCreate != scene::EActorType::UNKNOWN)
    {
        UIDrawActorResourceCreationMenu();
    }

    // Asset browser
    ImGui::Begin(ASSETS_BROWSER, nullptr, WindowFlags);
    {
        ResourceItemWidth = (ImGui::GetContentRegionAvailWidth() / GSceneEditorState.ResourceItemsPerRow) - 8;
        ResourceItemSize  = { ResourceItemWidth, ResourceItemWidth };
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Resource import
                if (ImGui::BeginMenu("Create asset"))
                {
                    if (ImGui::BeginMenu("Material"))
                    {
                        if (ImGui::MenuItem("Flat"))
                        {
                            GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::FLAT;
                        }
                        if (ImGui::MenuItem("Blinn Phong"))
                        {
                            GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::BLINN_PHONG;
                        }
                        if (ImGui::MenuItem("Blinn Phong Maps"))
                        {
                            GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::BLINN_PHONG_MAPS;
                        }
                        if (ImGui::MenuItem("PBR"))
                        {
                            GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::PBR;
                        }
                        if (ImGui::MenuItem("Textured PBR"))
                        {
                            GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::TEXTURED_PBR;
                        }

                        if (GSceneEditorState.TypeOfMaterialToCreate != scene::EMaterialType::NONE)
                        {
                            GSceneEditorState.PickedShader           = nullptr;
                            GSceneEditorState.bDisableCameraMovement = true;
                        }

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Actor"))
                    {
                        if (ImGui::MenuItem("Static mesh"))
                        {
                            GSceneEditorState.TypeOfActorToCreate    = scene::EActorType::STATIC_MESH;
                            GSceneEditorState.bDisableCameraMovement = true;
                        }
                        if (ImGui::MenuItem("Skybox"))
                        {
                            GSceneEditorState.TypeOfActorToCreate    = scene::EActorType::SKYBOX;
                            GSceneEditorState.bDisableCameraMovement = true;
                        }
                        if (ImGui::MenuItem("Terrain"))
                        {
                            GSceneEditorState.TypeOfActorToCreate    = scene::EActorType::TERRAIN;
                            GSceneEditorState.bDisableCameraMovement = true;
                            GSceneEditorState.GenericFloat2Param0[0] = 100;
                            GSceneEditorState.GenericFloat2Param0[1] = 100;
                            GSceneEditorState.GenericFloat2Param1[0] = 100;
                            GSceneEditorState.GenericFloat2Param1[1] = 100;
                            GSceneEditorState.GenericBoolParam0      = false;
                            GSceneEditorState.GenericIntParam0       = -1;
                            GSceneEditorState.GenericIntParam1       = 4;
                            GSceneEditorState.GenericFloatParam0     = 0.005f;
                            GSceneEditorState.GenericFloatParam1     = 1.0f;
                            GSceneEditorState.GenericFloatParam2     = 4.152f;
                            GSceneEditorState.GenericFloatParam3     = 0.122f;
                            GSceneEditorState.GenericFloatParam4     = 0.f;
                            GSceneEditorState.GenericFloatParam5     = 5.f;
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Checkbox("Materials", &GSceneEditorState.bShowMaterialAssets);
        ImGui::SameLine();
        ImGui::Checkbox("Actors", &GSceneEditorState.bShowActorAssets);

        ImGui::BeginChild("Asset scroll");
        {
            // Show all materials if checkbox is selected
            if (GSceneEditorState.bShowMaterialAssets)
            {
                static bool bMaterialEditorOpen = true;
                if (GSceneEditorState.EditedMaterial)
                {
                    ImGuiShowMaterialEditor(GSceneEditorState.EditedMaterial, &bMaterialEditorOpen);
                    if (!bMaterialEditorOpen)
                    {
                        GSceneEditorState.EditedMaterial = nullptr;
                        bMaterialEditorOpen              = true;
                    }
                }
                CurrentRowItemsCount = 0;
                ImGui::Text("Materials:");
                for (u32 i = 0; i < GEngine.GetMaterialsHolder().GetLength(); ++i, ++CurrentRowItemsCount)
                {
                    if (CurrentRowItemsCount > 0 && (CurrentRowItemsCount % GSceneEditorState.ResourceItemsPerRow) == 0)
                    {
                        CurrentRowItemsCount = 0;
                    }
                    else if (CurrentRowItemsCount > 0)
                    {
                        ImGui::SameLine(ResourceItemWidth * CurrentRowItemsCount + (4 * CurrentRowItemsCount), 2);
                    }

                    scene::CMaterial* Material = GEngine.GetMaterialsHolder().GetByIndex(i);
                    // Open material editor on left mouse click
                    if (ImGui::Button(*Material->GetName(), ResourceItemSize) && !GSceneEditorState.EditedMaterial)
                    {
                        GSceneEditorState.EditedMaterial = GEngine.GetMaterialsHolder().GetByIndex(i);
                    }

                    // Open context menu on right mouse click
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        GSceneEditorState.ClickedMaterialAsset   = Material;
                        GSceneEditorState.bDisableCameraMovement = true;
                    }
                }
            }

            if (GSceneEditorState.bShowActorAssets)
            {
                static bool bActorEditorOpen = true;
                if (GSceneEditorState.EditedActorAsset)
                {
                    ImGui::Begin("Actor resource details", &bActorEditorOpen);
                    if (!bActorEditorOpen)
                    {
                        GSceneEditorState.EditedActorAsset = nullptr;
                        bActorEditorOpen                   = true;
                    }
                    else
                    {
                        GSceneEditorState.EditedActorAsset->UIDrawActorDetails();
                    }

                    ImGui::End();
                }

                CurrentRowItemsCount = 0;
                ImGui::Text("Actors:");
                for (u32 i = 0; i < GEngine.GetActorsResources().GetLength(); ++i, ++CurrentRowItemsCount)
                {
                    if (CurrentRowItemsCount > 0 && (CurrentRowItemsCount % GSceneEditorState.ResourceItemsPerRow) == 0)
                    {
                        CurrentRowItemsCount = 0;
                    }
                    else if (CurrentRowItemsCount > 0)
                    {
                        ImGui::SameLine(ResourceItemWidth * CurrentRowItemsCount + (4 * CurrentRowItemsCount), 2);
                    }

                    scene::IActor* ActorAsset = GEngine.GetActorsResources().GetByIndex(i);
                    if (ImGui::Button(*ActorAsset->Name, ResourceItemSize) && !GSceneEditorState.ClickedActorAsset)
                    {
                        if (scene::IActor* ClickedActorAsset = GEngine.GetActorsResources().GetByIndex(i))
                        {
                            GSceneEditorState.EditedActorAsset = ClickedActorAsset;
                        }
                    }

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        // Set payload the pointer to ActorAsset
                        ImGui::SetDragDropPayload(ACTOR_ASSET_DRAG_TYPE, &ActorAsset->AssetId, sizeof(sole::uuid));

                        // Display preview (could be anything, e.g. when dragging an image we could decide to display
                        // the filename and a small preview of the image, etc.)
                        ImGui::Text(*ActorAsset->Name);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        GSceneEditorState.ClickedActorAsset = ActorAsset;
                    }
                }
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }
}

void UIDrawMeshImporter()
{
    if (!ImGui::IsPopupOpen(POPUP_WINDOW))
    {
        ImGui::OpenPopup(POPUP_WINDOW);
    }

    ImGui::SetNextWindowPos({ GSceneEditorState.Window->GetPosition().x + GSceneEditorState.Window->GetWidth() * 0.5f,
                              GSceneEditorState.Window->GetPosition().y + GSceneEditorState.Window->GetHeight() * 0.5f },
                            ImGuiCond_Always,
                            { 0.5f, 0.5f });
    ImGui::SetNextWindowSize({ 0, 0 });
    ImGui::BeginPopupModal(POPUP_WINDOW, nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::InputText("Mesh name (max 255)", GSceneEditorState.AssetNameBuffer, 255);
        ImGui::Checkbox("Flip UVs", &GSceneEditorState.GenericBoolParam0);

        static resources::EMeshImportStretegy MeshImportStrategy = resources::EMeshImportStretegy::SUBMESHES;
        if (ImGui::BeginListBox("Mesh import strategy"))
        {
            if (ImGui::Selectable("Submeshes", MeshImportStrategy == resources::EMeshImportStretegy::SUBMESHES))
            {
                MeshImportStrategy = resources::EMeshImportStretegy::SUBMESHES;
            }

            if (ImGui::Selectable("Single mesh", MeshImportStrategy == resources::EMeshImportStretegy::SINGLE_MESH))
            {
                MeshImportStrategy = resources::EMeshImportStretegy::SINGLE_MESH;
            }

            if (ImGui::Selectable("Split meshes", MeshImportStrategy == resources::EMeshImportStretegy::SPLIT_MESHES))
            {
                MeshImportStrategy = resources::EMeshImportStretegy::SPLIT_MESHES;
            }

            ImGui::EndListBox();
        }

        if (ImGui::Button("Import"))
        {
            GSceneEditorState.bAssetNameMissing       = false;
            GSceneEditorState.bFailedToImportResource = false;

            if (strnlen_s(GSceneEditorState.AssetNameBuffer, 256) == 0)
            {
                GSceneEditorState.bAssetNameMissing = true;
            }
            else
            {
                // Everything is ok, import the mesh and save it
                FDString                          MeshName = CopyToString(GSceneEditorState.AssetNameBuffer);
                FArray<resources::CMeshResource*> ImportedMeshes =
                  resources::ImportMesh(GSceneEditorState.PathToSelectedFile, MeshName, GSceneEditorState.GenericBoolParam0, MeshImportStrategy);

                ImportedMeshes.Free();

                // End mesh import
                GSceneEditorState.bIsImportingMesh       = false;
                GSceneEditorState.bDisableCameraMovement = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            GSceneEditorState.bIsImportingMesh       = false;
            GSceneEditorState.bDisableCameraMovement = false;
            ImGui::CloseCurrentPopup();
        }
        if (GSceneEditorState.bAssetNameMissing)
        {
            ImGui::Text("Please provide mesh name!");
        }
        if (GSceneEditorState.bFailedToImportResource)
        {
            ImGui::Text("Failed to import mesh"); // @TODO error code
        }
    }
    ImGui::EndPopup();
}

void UIDrawTextureImporter()
{
    if (!ImGui::IsPopupOpen(POPUP_WINDOW))
    {
        ImGui::OpenPopup(POPUP_WINDOW);
    }

    ImGui::SetNextWindowPos({ GSceneEditorState.Window->GetPosition().x + GSceneEditorState.Window->GetWidth() * 0.5f,
                              GSceneEditorState.Window->GetPosition().y + GSceneEditorState.Window->GetHeight() * 0.5f },
                            ImGuiCond_Always,
                            { 0.5f, 0.5f });
    ImGui::SetNextWindowSize({ 0, 0 });
    ImGui::BeginPopupModal(POPUP_WINDOW, nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::InputText("Texture name (max 255)", GSceneEditorState.AssetNameBuffer, 255);
        ImGui::Checkbox("Flip UVs", &GSceneEditorState.GenericBoolParam0);
        ImGui::Checkbox("Perform gamma correction", &GSceneEditorState.GenericBoolParam1);
        if (ImGui::Button("Import"))
        {
            GSceneEditorState.bAssetNameMissing       = false;
            GSceneEditorState.bFailedToImportResource = false;

            if (strnlen_s(GSceneEditorState.AssetNameBuffer, 256) == 0)
            {
                GSceneEditorState.bAssetNameMissing = true;
            }
            else
            {
                // Everything is ok, import the mesh and save it
                static char IMPORTED_TEXTURE_FILE_PATH[1024];
                sprintf_s(IMPORTED_TEXTURE_FILE_PATH, 1024, "assets/textures/%s.asset", GSceneEditorState.AssetNameBuffer);

                // Import the texture
                FDString                     TextureName     = CopyToString(GSceneEditorState.AssetNameBuffer);
                resources::CTextureResource* ImportedTexture = nullptr;

                if (GSceneEditorState.ImportingTextureType == EImportingTextureType::PNG)
                {
                    ImportedTexture = resources::ImportTexture(GSceneEditorState.PathToSelectedFile,
                                                               { IMPORTED_TEXTURE_FILE_PATH },
                                                               GSceneEditorState.GenericBoolParam1,
                                                               gpu::ETextureDataType::UNSIGNED_BYTE,
                                                               GSceneEditorState.GenericBoolParam0,
                                                               true,
                                                               TextureName);
                }
                else
                {
                    ImportedTexture = resources::ImportTexture(GSceneEditorState.PathToSelectedFile,
                                                               { IMPORTED_TEXTURE_FILE_PATH },
                                                               GSceneEditorState.GenericBoolParam1,
                                                               gpu::ETextureDataType::UNSIGNED_BYTE,
                                                               true,
                                                               true,
                                                               TextureName);
                }

                if (!ImportedTexture)
                {
                    GSceneEditorState.bFailedToImportResource = true;
                    return;
                }

                // Save it to file
                FILE* ImportedTextureFile;
                fopen_s(&ImportedTextureFile, IMPORTED_TEXTURE_FILE_PATH, "wb");
                ImportedTexture->SaveSynchronously(ImportedTextureFile);
                fclose(ImportedTextureFile);

                // Update engine resources database
                GEngine.AddTextureResource(ImportedTexture);

                // End texture import
                GSceneEditorState.bIsImportingTexture    = false;
                GSceneEditorState.bDisableCameraMovement = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            GSceneEditorState.bIsImportingTexture    = false;
            GSceneEditorState.bDisableCameraMovement = false;
            ImGui::CloseCurrentPopup();
        }
        if (GSceneEditorState.bAssetNameMissing)
        {
            ImGui::Text("Please provide texture name!");
        }
        if (GSceneEditorState.bFailedToImportResource)
        {
            ImGui::Text("Failed to import mesh"); // @TODO error code
        }
    }
    ImGui::EndPopup();
}

void UIDrawMeshContextMenu()
{
    UIOpenPopup(*GSceneEditorState.ClickedMeshResource->GetName());
    if (ImGui::Button("Migrate to latest version"))
    {
        GSceneEditorState.ClickedMeshResource->MigrateToLatestVersion();
        GSceneEditorState.ClickedMeshResource    = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Remove"))
    {
        GEngine.RemoveMeshResource(GSceneEditorState.ClickedMeshResource);
        GSceneEditorState.ClickedMeshResource    = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Close"))
    {
        GSceneEditorState.ClickedMeshResource    = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void UIDrawTextureContextMenu()
{
    UIOpenPopup(*GSceneEditorState.ClickedTextureResource->GetName());

    if (ImGui::Button("Migrate to latest version"))
    {
        GSceneEditorState.ClickedTextureResource->MigrateToLatestVersion();
        GSceneEditorState.ClickedTextureResource = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Remove"))
    {
        GEngine.RemoveTextureResource(GSceneEditorState.ClickedTextureResource);
        GSceneEditorState.ClickedTextureResource = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Close"))
    {
        GSceneEditorState.ClickedTextureResource = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void UIDrawSceneHierarchyWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    ImGui::Begin(SCENE_HIERARCHY, nullptr, WindowFlags);
    if (GSceneEditorState.CurrentCamera)
    {
        ImGui::Text("Camera position [%.4f, %.4f, %.4f]",
                    GSceneEditorState.CurrentCamera->GetPosition().x,
                    GSceneEditorState.CurrentCamera->GetPosition().y,
                    GSceneEditorState.CurrentCamera->GetPosition().z);
    }
    {
        if (GSceneEditorState.World)
        {
            auto ActorMap = GSceneEditorState.World->GetActorsMap();
            for (u32 i = 0; i < ActorMap.GetLength(); ++i)
            {
                auto* Actor = ActorMap.GetByIndex(i);
                if (Actor->Parent)
                {
                    continue; // The parent will handle it
                }
                if (auto* SelectedActor = Actor->UIDrawHierarchy())
                {
                    GSceneEditorState.CurrentlySelectedActor = SelectedActor;
                }
            }
        }
        ImGui::End();
    }
}

void UIDrawActorDetailsWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    if (ImGui::Begin(ACTOR_DETAILS, nullptr, WindowFlags))
    {
        if (GSceneEditorState.CurrentlySelectedActor)
        {
            ImGuizmo::Enable(true);
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetRect(
              GSceneEditorState.SceneWindowPos.x, GSceneEditorState.SceneWindowPos.y, GSceneEditorState.SceneWindowWidth, GSceneEditorState.SceneWindowHeight);

            ImGuizmo::BeginFrame();
            GSceneEditorState.CurrentlySelectedActor->DrawGizmos(GSceneEditorState.CurrentCamera);
            GSceneEditorState.Window->Prepare();
            GSceneEditorState.CurrentlySelectedActor->UIDrawActorDetails();
        }
        ImGui::End();
    }
}

void UIDrawFileDialog()
{
    if (GSceneEditorState.bShowFileDialog)
    {
        if (GSceneEditorState.FileDialog.HasSelected())
        {
            GSceneEditorState.OnFileSelected(GSceneEditorState.FileDialog.GetSelected());
            GSceneEditorState.bShowFileDialog = false;
        }
        else
        {
            GSceneEditorState.FileDialog.Display();
        }
    }
}

void ImportTexture(const std::filesystem::path& SelectedFilePath)
{
    const std::string& Extenstion = SelectedFilePath.extension().string();
    if (EqualIgnoreCase(Extenstion, ".png"))
    {
        GSceneEditorState.ImportingTextureType = EImportingTextureType::PNG;
    }
    else if (EqualIgnoreCase(Extenstion, ".jpg") || EqualIgnoreCase(Extenstion, ".jpeg") || EqualIgnoreCase(Extenstion, ".tga"))
    {
        GSceneEditorState.ImportingTextureType = EImportingTextureType::JPG;
    }
    else
    {
        return;
    }
    GSceneEditorState.PathToSelectedFile     = CopyToString(SelectedFilePath.string().c_str());
    GSceneEditorState.bIsImportingTexture    = true;
    GSceneEditorState.bDisableCameraMovement = true;
    Zero(GSceneEditorState.AssetNameBuffer, 256);
}

void ImportMesh(const std::filesystem::path& SelectedFilePath)
{
    GSceneEditorState.PathToSelectedFile     = CopyToString(SelectedFilePath.string().c_str());
    GSceneEditorState.bIsImportingMesh       = true;
    GSceneEditorState.bDisableCameraMovement = true;
    Zero(GSceneEditorState.AssetNameBuffer, 256);
}

void UIDrawMaterialCreationMenu()
{
    if (!ImGui::IsPopupOpen(POPUP_WINDOW))
    {
        ImGui::OpenPopup(POPUP_WINDOW);
    }

    ImGui::SetNextWindowPos({ GSceneEditorState.Window->GetPosition().x + GSceneEditorState.Window->GetWidth() * 0.5f,
                              GSceneEditorState.Window->GetPosition().y + GSceneEditorState.Window->GetHeight() * 0.5f },
                            ImGuiCond_Always,
                            { 0.5f, 0.5f });
    ImGui::SetNextWindowSize({ 500, 0 });
    ImGui::BeginPopupModal(POPUP_WINDOW, nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::InputText("Material name name (max 255)", GSceneEditorState.AssetNameBuffer, 255);
        ImGuiShadersPicker("Material shader", &GSceneEditorState.PickedShader);
        if (GSceneEditorState.PickedShader)
        {
            if (ImGui::Button("Create"))
            {
                if (strnlen_s(GSceneEditorState.AssetNameBuffer, 256) == 0)
                {
                    GSceneEditorState.bAssetNameMissing = true;
                }
                else
                {
                    auto CreatedMaterialPath = SPrintf("assets/materials/%s.asset", GSceneEditorState.AssetNameBuffer);

                    scene::CMaterial* CreatedMaterial;
                    switch (GSceneEditorState.TypeOfMaterialToCreate)
                    {
                    case scene::EMaterialType::FLAT:
                        GSceneEditorState.bDisableCameraMovement = true;
                        CreatedMaterial                          = new scene::CFlatMaterial{
                            sole::uuid4(), CopyToString(GSceneEditorState.AssetNameBuffer), CreatedMaterialPath, GSceneEditorState.PickedShader
                        };
                        break;
                    case scene::EMaterialType::BLINN_PHONG:
                        GSceneEditorState.bDisableCameraMovement = true;
                        CreatedMaterial                          = new scene::CBlinnPhongMaterial{
                            sole::uuid4(), CopyToString(GSceneEditorState.AssetNameBuffer), CreatedMaterialPath, GSceneEditorState.PickedShader
                        };
                        break;
                    case scene::EMaterialType::BLINN_PHONG_MAPS:
                        GSceneEditorState.bDisableCameraMovement = true;
                        CreatedMaterial                          = new scene::CBlinnPhongMapsMaterial{
                            sole::uuid4(), CopyToString(GSceneEditorState.AssetNameBuffer), CreatedMaterialPath, GSceneEditorState.PickedShader
                        };
                        break;
                    case scene::EMaterialType::PBR:
                        GSceneEditorState.bDisableCameraMovement = true;
                        CreatedMaterial                          = new scene::CPBRMaterial{
                            sole::uuid4(), CopyToString(GSceneEditorState.AssetNameBuffer), CreatedMaterialPath, GSceneEditorState.PickedShader
                        };
                        break;
                    case scene::EMaterialType::TEXTURED_PBR:
                        GSceneEditorState.bDisableCameraMovement = true;
                        CreatedMaterial                          = new scene::CTexturedPBRMaterial{
                            sole::uuid4(), CopyToString(GSceneEditorState.AssetNameBuffer), CreatedMaterialPath, GSceneEditorState.PickedShader
                        };    
                        break;

                    default:
                        assert(0);
                    }

                    GEngine.AddMaterialAsset(CreatedMaterial, GSceneEditorState.TypeOfMaterialToCreate, CreatedMaterialPath);
                    GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::NONE;
                    GSceneEditorState.PickedShader           = nullptr;
                    ImGui::CloseCurrentPopup();
                    GSceneEditorState.bDisableCameraMovement = false;
                }
            }
        }
        if (GSceneEditorState.PickedShader)
        {
            ImGui::SameLine();
        }
        if (ImGui::Button("Cancel"))
        {
            GSceneEditorState.TypeOfMaterialToCreate = scene::EMaterialType::NONE;
            GSceneEditorState.PickedShader           = nullptr;
            ImGui::CloseCurrentPopup();
            GSceneEditorState.bDisableCameraMovement = false;
        }
    }
    ImGui::EndPopup();
}

void UIDrawActorResourceCreationMenu()
{
    if (!ImGui::IsPopupOpen(POPUP_WINDOW))
    {
        ImGui::OpenPopup(POPUP_WINDOW);
    }

    ImGui::SetNextWindowPos({ GSceneEditorState.Window->GetPosition().x + GSceneEditorState.Window->GetWidth() * 0.5f,
                              GSceneEditorState.Window->GetPosition().y + GSceneEditorState.Window->GetHeight() * 0.5f },
                            ImGuiCond_Always,
                            { 0.5f, 0.5f });
    ImGui::SetNextWindowSize({ 500, 0 });
    ImGui::BeginPopupModal(POPUP_WINDOW, nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::InputText("Actor resource name name (max 255)", GSceneEditorState.AssetNameBuffer, 255);

        if (GSceneEditorState.TypeOfActorToCreate == scene::EActorType::SKYBOX)
        {
            ImGui::InputInt("Width", &GSceneEditorState.GenericIntParam0);
            ImGui::InputInt("Height", &GSceneEditorState.GenericIntParam1);
        }
        else if (GSceneEditorState.TypeOfActorToCreate == scene::EActorType::TERRAIN)
        {
            ImGui::InputFloat2("Grid size", GSceneEditorState.GenericFloat2Param0);
            ImGui::InputFloat2("Resolution", GSceneEditorState.GenericFloat2Param1);
            ImGui::Checkbox("Flat", &GSceneEditorState.GenericBoolParam0);
            ImGui::InputInt("Seed", &GSceneEditorState.GenericIntParam0);
            ImGui::InputInt("Octaves", &GSceneEditorState.GenericIntParam1);
            ImGui::InputFloat("Frequency", &GSceneEditorState.GenericFloatParam0);
            ImGui::InputFloat("Amplitude", &GSceneEditorState.GenericFloatParam1);
            ImGui::InputFloat("Lacunarity", &GSceneEditorState.GenericFloatParam2);
            ImGui::InputFloat("Persistence", &GSceneEditorState.GenericFloatParam3);
            ImGui::InputFloat("Min height", &GSceneEditorState.GenericFloatParam4);
            ImGui::InputFloat("Max height", &GSceneEditorState.GenericFloatParam5);
        }

        if (ImGui::Button("Create"))
        {
            if (strnlen_s(GSceneEditorState.AssetNameBuffer, 256) == 0)
            {
                GSceneEditorState.bAssetNameMissing = true;
            }
            else
            {
                auto CreatedActorPath = SPrintf("assets/actors/%s.asset", GSceneEditorState.AssetNameBuffer);

                scene::IActor* CreatedActor;
                switch (GSceneEditorState.TypeOfActorToCreate)
                {
                case scene::EActorType::STATIC_MESH:
                {
                    GSceneEditorState.bDisableCameraMovement = true;
                    CreatedActor                             = scene::CStaticMesh::CreateAsset(CopyToString(GSceneEditorState.AssetNameBuffer));
                    break;
                }
                case scene::EActorType::SKYBOX:
                {
                    if (GSceneEditorState.GenericIntParam0 > 0 || GSceneEditorState.GenericIntParam1 > 0)
                    {
                        GSceneEditorState.bDisableCameraMovement = true;
                        CreatedActor                             = scene::CSkybox::CreateAsset(CopyToString(GSceneEditorState.AssetNameBuffer),
                                                                   { (u32)GSceneEditorState.GenericIntParam0, (u32)GSceneEditorState.GenericIntParam1 });
                    }
                    break;
                }
                case scene::EActorType::TERRAIN:
                {

                    scene::FTerrainSettings TerrainSettings;
                    TerrainSettings.GridSize    = { GSceneEditorState.GenericFloat2Param0[0], GSceneEditorState.GenericFloat2Param0[1] };
                    TerrainSettings.Resolution  = { GSceneEditorState.GenericFloat2Param1[0], GSceneEditorState.GenericFloat2Param1[1] };
                    TerrainSettings.bFlatMesh   = GSceneEditorState.GenericBoolParam0;
                    TerrainSettings.Seed        = GSceneEditorState.GenericIntParam0;
                    TerrainSettings.Octaves     = GSceneEditorState.GenericIntParam1;
                    TerrainSettings.Frequency   = GSceneEditorState.GenericFloatParam0;
                    TerrainSettings.Amplitude   = GSceneEditorState.GenericFloatParam1;
                    TerrainSettings.Lacunarity  = GSceneEditorState.GenericFloatParam2;
                    TerrainSettings.Persistence = GSceneEditorState.GenericFloatParam3;
                    TerrainSettings.MinHeight   = GSceneEditorState.GenericFloatParam4;
                    TerrainSettings.MaxHeight   = GSceneEditorState.GenericFloatParam5;

                    GSceneEditorState.bDisableCameraMovement = true;
                    CreatedActor                             = scene::CTerrain::CreateAsset(CopyToString(GSceneEditorState.AssetNameBuffer), TerrainSettings);
                    break;
                }

                default:
                    assert(0);
                }

                CreatedActor->AssetId   = sole::uuid4();
                CreatedActor->AssetPath = CreatedActorPath;

                GEngine.AddActorAsset(CreatedActor);
                GSceneEditorState.TypeOfActorToCreate = scene::EActorType::UNKNOWN;
                ImGui::CloseCurrentPopup();
                GSceneEditorState.bDisableCameraMovement = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            GSceneEditorState.TypeOfActorToCreate = scene::EActorType::UNKNOWN;
            ImGui::CloseCurrentPopup();
            GSceneEditorState.bDisableCameraMovement = false;
        }
    }
    ImGui::EndPopup();
}

void UIDrawMaterialContextMenu()
{
    UIOpenPopup("Material actions");

    if (ImGui::Button("Remove"))
    {
        GEngine.RemoveMaterialAsset(GSceneEditorState.ClickedMaterialAsset);
        GSceneEditorState.ClickedMaterialAsset   = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Close"))
    {
        GSceneEditorState.ClickedMaterialAsset   = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void UIDrawActorAssetContextMenu()
{
    UIOpenPopup("Actor asset actions");

    if (ImGui::Button("Remove"))
    {
        GEngine.RemoveActorAsset(GSceneEditorState.ClickedActorAsset);
        GSceneEditorState.ClickedActorAsset      = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Close"))
    {
        GSceneEditorState.ClickedActorAsset      = nullptr;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void UIDrawCommonActorsWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    ImGui::Begin(COMMON_ACTORS, nullptr, WindowFlags);
    {
        // Center the item
        const float  HalfWindowWidth = ImGui::GetWindowSize().x / 2.f;
        const float  ImageItemWidth  = 64.f;
        const float  ImageItemPosX   = HalfWindowWidth - (ImageItemWidth / 2.f);
        const ImVec2 ImageItemSize{ ImageItemWidth, ImageItemWidth };

        // Directional light
        {
            ImGui::SetCursorPosX(ImageItemPosX);
            const auto TypeOfActorToSpawn = ECommonActorType::DIRECTIONAL_LIGHT;
            UIDrawDraggableImage("DragDropDirLight",
                                 GEngine.GetRenderer()->GetLightBulbTexture(),
                                 "Directional",
                                 ImageItemSize,
                                 COMMON_ACTOR_DRAG_TYPE,
                                 &TypeOfActorToSpawn,
                                 sizeof(TypeOfActorToSpawn));
        }

        // Spot light
        {
            ImGui::SetCursorPosX(ImageItemPosX);
            const auto TypeOfActorToSpawn = ECommonActorType::SPOT_LIGHT;
            UIDrawDraggableImage("DragDropSpotLight",
                                 GEngine.GetRenderer()->GetLightBulbTexture(),
                                 "Spot",
                                 ImageItemSize,
                                 COMMON_ACTOR_DRAG_TYPE,
                                 &TypeOfActorToSpawn,
                                 sizeof(TypeOfActorToSpawn));
        }

        // Point light
        {
            ImGui::SetCursorPosX(ImageItemPosX);
            const auto TypeOfActorToSpawn = ECommonActorType::POINT_LIGHT;
            UIDrawDraggableImage("DragDropPointLight",
                                 GEngine.GetRenderer()->GetLightBulbTexture(),
                                 "Point",
                                 ImageItemSize,
                                 COMMON_ACTOR_DRAG_TYPE,
                                 &TypeOfActorToSpawn,
                                 sizeof(TypeOfActorToSpawn));
        }

        ImGui::End();
    }
}

void UIDrawDraggableImage(const char*    InDragDropId,
                          gpu::CTexture* InTexture,
                          const char*    InLabel,
                          const ImVec2&  InSize,
                          const char*    InDragDataType,
                          const void*    InData,
                          const u32&     InDataSize)
{
    ImGui::BeginGroup();
    ImGui::PushID(InDragDropId);
    InTexture->ImGuiImageButton(InSize);
    ImGui::PopID();

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        // Set payload the pointer to ActorAsset
        ImGui::SetDragDropPayload(InDragDataType, InData, InDataSize);

        // Display preview
        InTexture->ImGuiDrawToImage({ 16, 16 });
        ImGui::EndDragDropSource();
    }

    ImGui::TextCenter(InLabel);
    ImGui::EndGroup();
    ImGui::Spacing();
}

void LoadWorld(const std::filesystem::path& SelectedFilePath)
{
    GSceneEditorState.WorldFilePath = CopyToString(SelectedFilePath.string().c_str());
    GSceneEditorState.World         = scene::LoadWorldFromJSONFile(GSceneEditorState.WorldFilePath);
}

void UIDrawHelpWindow() {}

void UIDrawStatsWindow()
{
    static FDString MaxVideoMemLabel = SPrintf("Available: %f MB", float(gpu::GGPUStatus.TotalAvailableVideoMemoryKB) / 1024.f);

    if (GSceneEditorState.bShowingStatsWindow)
    {
        ImGui::SetNextWindowSize({ 0, 0 });
        ImGui::Begin("Statistics", &GSceneEditorState.bShowingStatsWindow);
        ImGui::PlotHistogram("Memory usage (MB) (not accurate yet)",
                             GSceneEditorState.VideoMemoryUsage,
                             GSceneEditorState.NumVideoMemoryUsageSamples,
                             0,
                             *MaxVideoMemLabel,
                             0.0f,
                             float(gpu::GGPUStatus.TotalAvailableVideoMemoryKB) / 1024.f,
                             ImVec2(0, 100));

        ImGui::Spacing();

        ImGui::PlotHistogram("Num draw calls", GSceneEditorState.NumDrawCalls, GSceneEditorState.NumDrawCallSamples, 0, NULL, 0.0f, 2000, ImVec2(0, 100));

        ImGui::Spacing();

        ImGui::PlotLines("Frame time (ms)", GSceneEditorState.FrameTimes, GSceneEditorState.NumFrameTimesSamples, 0, NULL, 0.0f, 120, ImVec2(0, 100));

        ImGui::End();
    }
}

void UIDrawSettingsWindows()
{
    if (GSceneEditorState.bShowingRendererSettinsWindow)
    {
        if (!GEngine.GetRenderer()->UIDrawSettingsWindow())
        {
            GSceneEditorState.bShowingRendererSettinsWindow = false;
        }
    }
}