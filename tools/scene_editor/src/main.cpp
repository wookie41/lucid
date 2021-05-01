#include "engine/engine.hpp"

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/viewport.hpp"
#include "devices/gpu/shaders_manager.hpp"

#include "misc/basic_shapes.hpp"

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "platform/util.hpp"
#include "platform/platform.hpp"

#include "scene/camera.hpp"
#include "scene/forward_renderer.hpp"
#include "scene/blinn_phong_material.hpp"
#include "scene/render_scene.hpp"
#include "scene/actors/static_mesh.hpp"
#include "scene/actors/lights.hpp"
#include "scene/flat_material.hpp"
#include "scene/world.hpp"
#include "scene/actors/skybox.hpp"

#include "resources/texture_resource.hpp"
#include "resources/mesh_resource.hpp"

#include "glm/gtc/quaternion.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"

#include "schemas/json.hpp"
#include "misc/actor_thumbs.hpp"

#include <algorithm>
#include <stdio.h>
#include <devices/gpu/texture_enums.hpp>

using namespace lucid;

bool EqualIgnoreCase(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == tolower(b); });
}

constexpr char EDITOR_WINDOW[] = "Lucid Editor";
constexpr char DOCKSPACE_WINDOW[] = "Dockspace";
constexpr char SCENE_VIEWPORT[] = "Scene";
constexpr char RESOURCES_BROWSER[] = "Resources browser";
constexpr char SCENE_HIERARCHY[] = "Scene hierarchy";
constexpr char MAIN_DOCKSPACE[] = "MainDockSpace";
constexpr char POPUP_WINDOW[] = "PopupWindow";
constexpr char ACTOR_DETAILS[] = "Actor Details";

enum class EImportingTextureType : u8
{
    JPG,
    PNG
};

struct FSceneEditorState
{
    platform::CWindow* Window;
    ImGuiContext* ImGuiContext;
    ImGuiWindow* ImSceneWindow;

    ImGuiID MainDockId = 0;
    ImGuiID SceneDockId = 0;

    u16 EditorWindowWidth = 1280;
    u16 EditorWindowHeight = 720;
    float EditorWindowAspectRatio = 1280.f / 720.f;

    /** These are updated when drawing ui */
    float SceneWindowWidth;
    float SceneWindowHeight;
    ImVec2 SceneWindowPos;

    /** User interface stuff */
    bool bIsAnyUIWidgetHovered = false;
    bool bIsFileDialogOpen = false;

    scene::CWorld* World = nullptr;

    scene::IActor* CurrentlyDraggedActor = nullptr;
    float DistanceToCurrentlyDraggedActor = 0;

    scene::CCamera PerspectiveCamera{ scene::ECameraMode::PERSPECTIVE };
    scene::CCamera* CurrentCamera = nullptr;

    /** File dialog related things, used e.x. when importing assets */
    bool bShowFileDialog = false;
    ImGui::FileBrowser FileDialog;
    void (*OnFileSelected)(const std::filesystem::path&);

    /** Variables used when importing an asset to the engine */

    char AssetNameBuffer[256];
    FDString PathToSelectedFile{ "" };
    EImportingTextureType ImportingTextureType;

    bool bAssetNameMissing = false;
    bool bIsImportingMesh = false;
    bool bIsImportingTexture = false;
    bool bFailedToImportResource = false;
    bool bShowMeshContextMenu = false;
    bool bShowTextureContextMenu = false;

    resources::CMeshResource* ClickedMeshResource = nullptr;
    resources::CTextureResource* ClickedTextureResource = nullptr;

    bool bDisableCameraMovement = false;

} GSceneEditorState;

void InitializeSceneEditor();

void HandleCameraMovement(const float& DeltaTime);
void HandleActorDrag();

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

void ImportTexture(const std::filesystem::path& SelectedFilePath);
void ImportMesh(const std::filesystem::path& SelectedFilePath);

int main(int argc, char** argv)
{
    InitializeSceneEditor();

    gpu::CVertexArray* UnitCubeVAO = misc::CreateCubeVAO();
    gpu::CVertexArray* QuadVAO = misc::CreateQuadVAO();

    // Load textures and meshes used in the demo scene

    gpu::SetClearColor(BlackColor);

    GSceneEditorState.World = scene::LoadWorldFromJSONFile("assets/worlds/Demo.asset");

    bool IsRunning = true;

    real now = platform::GetCurrentTimeSeconds();
    real last = 0;
    real dt = 0;

    scene::FRenderView RenderView;
    RenderView.Camera = GSceneEditorState.CurrentCamera; // @TODO is this needed???
    RenderView.Viewport = { 0, 0, 1920, 1080 }; // @TODO Engine level variable

    while (IsRunning)
    {
        platform::Update();
        last = now;
        now = platform::GetCurrentTimeSeconds();
        dt += now - last;
        ReadEvents(GSceneEditorState.Window);

        if (WasKeyPressed(SDLK_ESCAPE))
        {
            IsRunning = false;
            break;
        }

        while (dt > platform::SimulationStep)
        {
            dt -= platform::SimulationStep;
            HandleCameraMovement(dt);
        }

        // Render the scene to off-screen framebuffer
        GSceneEditorState.Window->Prepare();
        GEngine.GetRenderer()->Render(GSceneEditorState.World->MakeRenderScene(GSceneEditorState.CurrentCamera), &RenderView);

        GSceneEditorState.Window->ImgUiStartNewFrame();
        {
            UISetupDockspace();
            ImGui::ShowDemoWindow();
            UIDrawSceneWindow();
            UIDrawResourceBrowserWindow();
            UIDrawSceneHierarchyWindow();
            UIDrawActorDetailsWindow();
            UIDrawFileDialog();
        }

        GSceneEditorState.Window->Clear();
        GSceneEditorState.Window->ImgUiDrawFrame();
        GSceneEditorState.Window->Swap();

        // Allow ImGui viewports to update
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        HandleActorDrag();
    }

    GSceneEditorState.World->SaveToJSONFile("assets/worlds/Demo.asset");
    GEngine.Shutdown();
    return 0;
}

void InitializeSceneEditor()
{
    // Create the window for the editor

    FEngineConfig EngineConfig;
    EngineConfig.bHotReloadShaders = true;

    GEngine.InitEngine(EngineConfig);

    platform::FWindowDefiniton EditorWindow;
    EditorWindow.title = "Lucid Editor";
    EditorWindow.X = 100;
    EditorWindow.Y = 100;
    EditorWindow.Width = GSceneEditorState.EditorWindowWidth;
    EditorWindow.Height = GSceneEditorState.EditorWindowHeight;
    EditorWindow.sRGBFramebuffer = true;
    EditorWindow.bHidden = false;

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
            ImGui::DockBuilderSetNodeSize(
              GSceneEditorState.MainDockId,
              { (float)GSceneEditorState.EditorWindowWidth, (float)GSceneEditorState.EditorWindowHeight });

            // Split the dockspace
            ImGuiID ResourceBrowserWindowDockId;
            ImGuiID SceneHierarchyDockId;
            ImGuiID ActorDetailsDockId;
            ImGuiID SceneWindowDockId;
            SceneWindowDockId =
              ImGui::DockBuilderSplitNode(GSceneEditorState.MainDockId, ImGuiDir_Left, 0.75f, nullptr, &SceneHierarchyDockId);
            SceneWindowDockId =
              ImGui::DockBuilderSplitNode(SceneWindowDockId, ImGuiDir_Up, 0.7f, nullptr, &ResourceBrowserWindowDockId);
            SceneHierarchyDockId =
              ImGui::DockBuilderSplitNode(SceneHierarchyDockId, ImGuiDir_Up, 0.5f, nullptr, &ActorDetailsDockId);

            // Attach windows to dockspaces
            ImGui::DockBuilderDockWindow(SCENE_VIEWPORT, SceneWindowDockId);
            ImGui::DockBuilderDockWindow(RESOURCES_BROWSER, ResourceBrowserWindowDockId);
            ImGui::DockBuilderDockWindow(SCENE_HIERARCHY, SceneHierarchyDockId);
            ImGui::DockBuilderDockWindow(ACTOR_DETAILS, ActorDetailsDockId);

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
    GSceneEditorState.PerspectiveCamera.AspectRatio =
    GSceneEditorState.ImSceneWindow->Size.x / GSceneEditorState.ImSceneWindow->Size.y;
    GSceneEditorState.PerspectiveCamera.Position = { 0, 0, 3 };
    GSceneEditorState.PerspectiveCamera.Yaw = -90.f;
    GSceneEditorState.PerspectiveCamera.UpdateCameraVectors();

    GSceneEditorState.CurrentCamera = &GSceneEditorState.PerspectiveCamera;
    
    GSceneEditorState.Window->ImgUiDrawFrame();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void HandleCameraMovement(const float& DeltaTime)
{
    // Check if we're inside the scene window
    const ImVec2 MousePositionAbs = ImGui::GetMousePos();
    const ImVec2 SceneWindowPos = GSceneEditorState.SceneWindowPos;
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
        GSceneEditorState.CurrentlyDraggedActor = nullptr;
        GSceneEditorState.CurrentCamera->AddRotation(-MousePos.DeltaX * 17.5 * DeltaTime, MousePos.DeltaY * 17.5 * DeltaTime);
    }
}

void HandleActorDrag()
{
    const ImVec2 MousePositionAbs = ImGui::GetMousePos();
    const ImVec2 SceneWindowPos = GSceneEditorState.SceneWindowPos;
    const ImVec2 MousePosRelative = { MousePositionAbs.x - SceneWindowPos.x, MousePositionAbs.y - SceneWindowPos.y };

    // Check if we're outside the scene window
    if (MousePosRelative.x < 0 || MousePosRelative.x > GSceneEditorState.SceneWindowWidth || MousePositionAbs.y < 0 ||
        MousePosRelative.y > GSceneEditorState.SceneWindowHeight)
    {
        return;
    }

    // Check if the scene window is not obscured by some other widget
    if (ImGui::GetFocusID() != GSceneEditorState.ImSceneWindow->ID && ImGui::GetFocusID() != GSceneEditorState.SceneDockId)
    {
        return;
    }

    if (IsMouseButtonPressed(LEFT))
    {
        // Check if've hit something based on the hit map generated by the renderer
        const scene::FHitMap& CachedHitMap = GEngine.GetRenderer()->GetCachedHitMap();

        // Adjust mouse postion based on hitmap texture size
        const float RatioX = MousePosRelative.x / GSceneEditorState.SceneWindowWidth;
        const float RatioY = MousePosRelative.y / GSceneEditorState.SceneWindowHeight;

        const glm::vec2 AdjustedMousePos = { ((float)CachedHitMap.Width) * RatioX, ((float)CachedHitMap.Height) * RatioY };

        scene::IActor* ClickedActor = GSceneEditorState.World->GetActorById(CachedHitMap.GetIdAtMousePositon(AdjustedMousePos));

        // Ignore skyboxes
        if (ClickedActor != nullptr && ClickedActor->GetActorType() != scene::EActorType::SKYBOX)
        {
            if (GSceneEditorState.CurrentlyDraggedActor == nullptr)
            {
                // Remember the actor that we hit and how far from the camera it was on the z axis
                glm::vec4 ActorPosView = GSceneEditorState.CurrentCamera->GetViewMatrix() * glm::vec4{ ClickedActor->Transform.Translation, 1 };
                GSceneEditorState.DistanceToCurrentlyDraggedActor = ActorPosView.z;
                GSceneEditorState.CurrentlyDraggedActor = ClickedActor;
            }

            // Actor pos from world to view space
            glm::vec4 ActorPosView = GSceneEditorState.CurrentCamera->GetViewMatrix() *
                                     glm::vec4{ GSceneEditorState.CurrentlyDraggedActor->Transform.Translation, 1 };

            // Get the mouse ray in view space from mouse pos
            const glm::vec2 MouseRayNDC = 2.f * glm::vec2{ MousePosRelative.x / GSceneEditorState.SceneWindowWidth,
                                                           1 - (MousePosRelative.y / GSceneEditorState.SceneWindowHeight) } -
                                          1.f;

            const glm::vec4 MouseRayClip{ MouseRayNDC, -1, 1 };
            const glm::vec4 MouseRayView = glm::inverse(GSceneEditorState.CurrentCamera->GetProjectionMatrix()) * MouseRayClip;

            // Calculate actor's position to match mouse view space x/y pos, preserving z
            const float ActorMidPoint = GSceneEditorState.CurrentlyDraggedActor->GetVerticalMidPoint();
            
            ActorPosView.x = -MouseRayView.x * GSceneEditorState.DistanceToCurrentlyDraggedActor;
            ActorPosView.y = -MouseRayView.y * GSceneEditorState.DistanceToCurrentlyDraggedActor - ActorMidPoint;

            // Update actor's position
            GSceneEditorState.CurrentlyDraggedActor->Transform.Translation =
              (glm::inverse(GSceneEditorState.CurrentCamera->GetViewMatrix()) * ActorPosView);
        }
    }
}

void UISetupDockspace()
{
    // Create the dockspace window which will occupy the whole editor window
    ImGuiWindowFlags DockspaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    DockspaceWindowFlags |=
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    DockspaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin(DOCKSPACE_WINDOW, nullptr, DockspaceWindowFlags);
    ImGui::PopStyleVar(2);

    ImGui::DockSpace(GSceneEditorState.MainDockId, ImVec2(0.0f, 0.0f));
    ImGui::End();
}

void UIDrawSceneWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin(SCENE_VIEWPORT, nullptr, WindowFlags);
    {
        // Update scene editor state so it can be used later in the frame
        const ImVec2 SceneWindowPos = ImGui::GetWindowPos();
        const ImVec2 WindowContentMin = ImGui::GetWindowContentRegionMin();
        const ImVec2 WindowContentMax = ImGui::GetWindowContentRegionMax();

        GSceneEditorState.SceneWindowWidth = WindowContentMax.x - WindowContentMin.x;
        GSceneEditorState.SceneWindowHeight = WindowContentMax.y - WindowContentMin.y;

        // This gets the position of the content area, excluding title bars and etc.
        GSceneEditorState.SceneWindowPos = WindowContentMin;
        GSceneEditorState.SceneWindowPos.x += SceneWindowPos.x;
        GSceneEditorState.SceneWindowPos.y += SceneWindowPos.y;

        // Draw the rendered scene into an image
        GEngine.GetRenderer()->GetResultFramebuffer()->ImGuiDrawToImage({ GSceneEditorState.SceneWindowWidth, GSceneEditorState.SceneWindowHeight });
    }
    ImGui::PopStyleVar(1);
    ImGui::End();
}

void UIDrawResourceBrowserWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
    ImGui::Begin(RESOURCES_BROWSER, nullptr, WindowFlags);
    {
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
                        GSceneEditorState.OnFileSelected = &ImportMesh;
                        GSceneEditorState.FileDialog.Open();
                    }

                    if (ImGui::MenuItem("Texture"))
                    {
                        GSceneEditorState.FileDialog.SetTitle("Select a texture file");
                        GSceneEditorState.FileDialog.SetTypeFilters({ ".png", ".jpg", ".jpeg" });
                        GSceneEditorState.OnFileSelected = &ImportTexture;
                        GSceneEditorState.bShowFileDialog = true;
                        GSceneEditorState.FileDialog.Open();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenuBar();

        // Resources browser
        ImGui::BeginChild("Scrolling");
        {
            const int ResourceItemsPerRow = 8;
            const float ResourceItemWidth = (ImGui::GetContentRegionAvailWidth() / ResourceItemsPerRow) - 4;
            const ImVec2 ResourceItemSize{ ResourceItemWidth, ResourceItemWidth };

            int CurrentRowItemsCount = 0;

            // Draw texture resource items
            ImGui::Text("Textures:");
            for (int i = 0; i < GEngine.GetTexturesHolder().Length(); ++i)
            {
                resources::CTextureResource* TextureResource = GEngine.GetTexturesHolder().GetByIndex(i);
                if (CurrentRowItemsCount > 0 && (CurrentRowItemsCount % ResourceItemsPerRow) == 0)
                {
                    CurrentRowItemsCount = 0;
                }
                else if (CurrentRowItemsCount > 0)
                {
                    ImGui::SameLine(ResourceItemWidth * CurrentRowItemsCount + (4 * CurrentRowItemsCount), 2);
                }

                if (TextureResource && TextureResource->TextureHandle)
                {
                    ImGui::BeginGroup();
                    TextureResource->TextureHandle->ImGuiImageButton(ResourceItemSize);
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        GSceneEditorState.bShowTextureContextMenu = true;
                        GSceneEditorState.ClickedTextureResource = TextureResource;
                    }
                    ImGui::Button(*TextureResource->GetName(), { ResourceItemWidth, 0 });
                    ImGui::EndGroup();
                    ++CurrentRowItemsCount;
                }

            }

            ImGui::Spacing();

            // Draw meshes resource items
            CurrentRowItemsCount = 0;
            ImGui::Text("Meshes:");

            for (int i = 0; i < GEngine.GetMeshesHolder().Length(); ++i)
            {

                resources::CMeshResource* MeshResource = GEngine.GetMeshesHolder().GetByIndex(i);
                if (CurrentRowItemsCount > 0 && (CurrentRowItemsCount % ResourceItemsPerRow) == 0)
                {
                    CurrentRowItemsCount = 0;
                }
                else if (CurrentRowItemsCount > 0)
                {
                    ImGui::SameLine(ResourceItemWidth * CurrentRowItemsCount + (4 * CurrentRowItemsCount), 2);
                }

                if (MeshResource && MeshResource->Thumb)
                {
                    ImGui::BeginGroup();
                    MeshResource->Thumb->ImGuiImageButton(ResourceItemSize);
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        GSceneEditorState.bShowMeshContextMenu = true;
                        GSceneEditorState.ClickedMeshResource = MeshResource;
                    }

                    ImGui::Button(*MeshResource->GetName(), { ResourceItemWidth, 0 });
                    ImGui::EndGroup();
                    ++CurrentRowItemsCount;
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    if (GSceneEditorState.bIsImportingMesh)
    {
        UIDrawMeshImporter();
    }
    else if (GSceneEditorState.bIsImportingTexture)
    {
        UIDrawTextureImporter();
    }
    else if (GSceneEditorState.bShowMeshContextMenu)
    {
        UIDrawMeshContextMenu();
    }
    else if (GSceneEditorState.bShowTextureContextMenu)
    {
        UIDrawTextureContextMenu();
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
        if (ImGui::Button("Import"))
        {
            GSceneEditorState.bAssetNameMissing = false;
            GSceneEditorState.bFailedToImportResource = false;

            if (strnlen_s(GSceneEditorState.AssetNameBuffer, 256) == 0)
            {
                GSceneEditorState.bAssetNameMissing = true;
            }
            else
            {
                // Everything is ok, import the mesh and save it
                static char IMPORTED_MESH_FILE_PATH[1024];
                sprintf_s(IMPORTED_MESH_FILE_PATH, 1024, "assets/meshes/%s.asset", GSceneEditorState.AssetNameBuffer);

                // Import the mesh
                FDString MeshName = CopyToString(GSceneEditorState.AssetNameBuffer);
                resources::CMeshResource* ImportedMesh =
                  resources::ImportMesh(GSceneEditorState.PathToSelectedFile, { IMPORTED_MESH_FILE_PATH }, MeshName);

                if (!ImportedMesh)
                {
                    GSceneEditorState.bFailedToImportResource = true;
                    return;
                }

                ImportedMesh->LoadDataToVideoMemorySynchronously();
                ImportedMesh->Thumb = GEngine.ThumbsGenerator->GenerateMeshThumb(256, 256, ImportedMesh);

                // Save it to file
                FILE* ImportedMeshFile;
                fopen_s(&ImportedMeshFile, IMPORTED_MESH_FILE_PATH, "wb");
                ImportedMesh->SaveSynchronously(ImportedMeshFile);
                fclose(ImportedMeshFile);

                // Update engine resources database
                GEngine.AddMeshResource(ImportedMesh, GSceneEditorState.PathToSelectedFile);

                // End mesh import
                GSceneEditorState.bIsImportingMesh = false;
                GSceneEditorState.bDisableCameraMovement = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            GSceneEditorState.bIsImportingMesh = false;
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
        if (ImGui::Button("Import"))
        {
            GSceneEditorState.bAssetNameMissing = false;
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
                FDString TextureName = CopyToString(GSceneEditorState.AssetNameBuffer);
                resources::CTextureResource* ImportedTexture = nullptr;

                if (GSceneEditorState.ImportingTextureType == EImportingTextureType::PNG)
                {
                    ImportedTexture = resources::ImportTexture(GSceneEditorState.PathToSelectedFile,
                                                               { IMPORTED_TEXTURE_FILE_PATH },
                                                               true,
                                                               gpu::ETextureDataType::UNSIGNED_BYTE,
                                                               true,
                                                               true,
                                                               TextureName);
                }
                else
                {
                    ImportedTexture = resources::ImportTexture(GSceneEditorState.PathToSelectedFile,
                                                               { IMPORTED_TEXTURE_FILE_PATH },
                                                               true,
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
                GEngine.AddTextureResource(ImportedTexture, GSceneEditorState.PathToSelectedFile);

                // End texture import
                GSceneEditorState.bIsImportingTexture = false;
                GSceneEditorState.bDisableCameraMovement = true;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            GSceneEditorState.bIsImportingTexture = false;
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
    ImGui::Text("Mesh resource actions:");

    if (ImGui::Button("Migrate to latest version"))
    {
        GSceneEditorState.ClickedMeshResource->MigrateToLatestVersion();
        GSceneEditorState.bShowMeshContextMenu = false;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Remove"))
    {
        GEngine.RemoveMeshResource(GSceneEditorState.ClickedMeshResource);
        GSceneEditorState.bShowMeshContextMenu = false;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Close"))
    {
        GSceneEditorState.bShowMeshContextMenu = false;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void UIDrawTextureContextMenu()
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
    ImGui::Text("Texture resource actions:");

    if (ImGui::Button("Migrate to latest version"))
    {
        GSceneEditorState.ClickedTextureResource->MigrateToLatestVersion();
        GSceneEditorState.bShowTextureContextMenu = false;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Remove"))
    {
        GEngine.RemoveTextureResource(GSceneEditorState.ClickedTextureResource);
        GSceneEditorState.bShowTextureContextMenu = false;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Close"))
    {
        GSceneEditorState.bShowTextureContextMenu = false;
        GSceneEditorState.bDisableCameraMovement = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void UIDrawSceneHierarchyWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    ImGui::Begin(SCENE_HIERARCHY, nullptr, WindowFlags);
    {
    }
    ImGui::End();
}

void UIDrawActorDetailsWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    ImGui::Begin(ACTOR_DETAILS, nullptr, WindowFlags);
    {
        if (GSceneEditorState.CurrentlyDraggedActor)
        {
            GSceneEditorState.CurrentlyDraggedActor->UIDrawActorDetails();
        }
    }
    ImGui::End();
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
    else if (EqualIgnoreCase(Extenstion, ".jpg"))
    {
        GSceneEditorState.ImportingTextureType = EImportingTextureType::JPG;
    }
    else
    {
        return;
    }
    GSceneEditorState.PathToSelectedFile = CopyToString(SelectedFilePath.string().c_str());
    GSceneEditorState.bIsImportingTexture = true;
    GSceneEditorState.bDisableCameraMovement = true;
    Zero(GSceneEditorState.AssetNameBuffer, 256);
}

void ImportMesh(const std::filesystem::path& SelectedFilePath)
{
    GSceneEditorState.PathToSelectedFile = CopyToString(SelectedFilePath.string().c_str());
    GSceneEditorState.bIsImportingMesh = true;
    GSceneEditorState.bDisableCameraMovement = true;
    Zero(GSceneEditorState.AssetNameBuffer, 256);
}
