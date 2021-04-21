#include <filesystem>
#include <devices/gpu/texture_enums.hpp>
#include <scene/material.hpp>

#include "engine.hpp"
#include "common/collections.hpp"

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
#include "schemas/types.hpp"
#include "schemas/json.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"

using namespace lucid;

constexpr char EDITOR_WINDOW[] = "Lucid Editor";
constexpr char DOCKSPACE_WINDOW[] = "Dockspace";
constexpr char SCENE_VIEWPORT[] = "Scene";
constexpr char ASSET_BROWSER[] = "Asset browser";
constexpr char SCENE_HIERARCHY[] = "Scene hierarchy";
constexpr char MAIN_DOCKSPACE[] = "MainDockSpace";

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
    scene::CRenderer* Renderer = nullptr; // @TODO this should be an engine variable

    scene::IActor* CurrentlyDraggedActor = nullptr;
    float DistanceToCurrentlyDraggedActor = 0;

    scene::CCamera PerspectiveCamera{ scene::ECameraMode::PERSPECTIVE };
    scene::CCamera* CurrentCamera = nullptr;

    bool bShowFileDialog = false;
    ImGui::FileBrowser FileDialog;
    void (*OnFileSelected)(const std::filesystem::path&);

    char AssetsBaseDir[] = "assets/";

} GSceneEditorState;

void InitializeSceneEditor();

void HandleCameraMovement(const float& DeltaTime);
void HandleActorDrag();

void UISetupDockspace();
void UIDrawSceneWindow();
void UIDrawAssetBrowserWindow();
void UIDrawSceneHierarchyWindow();
void UIDrawFileDialog();

void ImportTexture(const std::filesystem::path& SelectedFilePath);
void ImportMesh(const std::filesystem::path& SelectedFilePath);

int main(int argc, char** argv)
{
    FEngineConfig EngineConfig;
    EngineConfig.bHotReloadShaders = true;

    InitEngine(EngineConfig);
    resources::InitTextures();

    InitializeSceneEditor();

    gpu::CVertexArray* UnitCubeVAO = misc::CreateCubeVAO();
    gpu::CVertexArray* QuadVAO = misc::CreateQuadVAO();

    FShadersDataBase ShadersDatabase;
    ReadFromJSONFile(ShadersDatabase, "assets/shaders/shaders_database.json");

    gpu::GShadersManager.LoadShadersDatabase(ShadersDatabase);

    // Load textures and meshes used in the demo scene

    FString BrickDiffuseTextureFilePath{ "assets/textures/BrickDiffuse.asset" };
    FString BrickNormalTextureFilePath{ "assets/textures/BrickNormal.asset" };
    FString WoodDiffuseTextureFilePath{ "assets/textures/WoodDiffuse.asset" };
    FString BlankTextureFilePath{ "assets/textures/Blank.asset" };
    FString ToyboxNormalTextureFilePath{ "assets/textures/ToyboxNormal.asset" };
    FString ToyboxDisplacementTextureFilePath{ "assets/textures/ToyboxDisplacement.asset" };
    FString LightBulbTextureFilePath{ "assets/textures/LightBulb.asset" };

    resources::CTextureResource* BrickDiffuseTextureResource = resources::LoadTexture(BrickDiffuseTextureFilePath);
    BrickDiffuseTextureResource->LoadDataToMainMemorySynchronously();
    BrickDiffuseTextureResource->LoadDataToVideoMemorySynchronously();
    BrickDiffuseTextureResource->FreeMainMemory();

    resources::CTextureResource* BrickNormalTextureResource = resources::LoadTexture(BrickNormalTextureFilePath);
    BrickNormalTextureResource->LoadDataToMainMemorySynchronously();
    BrickNormalTextureResource->LoadDataToVideoMemorySynchronously();
    BrickNormalTextureResource->FreeMainMemory();

    resources::CTextureResource* WoodDiffuseTextureResource = resources::LoadTexture(WoodDiffuseTextureFilePath);
    WoodDiffuseTextureResource->LoadDataToMainMemorySynchronously();
    WoodDiffuseTextureResource->LoadDataToVideoMemorySynchronously();
    WoodDiffuseTextureResource->FreeMainMemory();

    resources::CTextureResource* BlankTextureResource = resources::LoadTexture(BlankTextureFilePath);
    BlankTextureResource->LoadDataToMainMemorySynchronously();
    BlankTextureResource->LoadDataToVideoMemorySynchronously();
    BlankTextureResource->FreeMainMemory();

    resources::CTextureResource* ToyboxNormalTextureResource = resources::LoadTexture(ToyboxNormalTextureFilePath);
    ToyboxNormalTextureResource->LoadDataToMainMemorySynchronously();
    ToyboxNormalTextureResource->LoadDataToVideoMemorySynchronously();
    ToyboxNormalTextureResource->FreeMainMemory();

    resources::CTextureResource* ToyboxDisplacementTextureResource = resources::LoadTexture(ToyboxDisplacementTextureFilePath);
    ToyboxDisplacementTextureResource->LoadDataToMainMemorySynchronously();
    ToyboxDisplacementTextureResource->LoadDataToVideoMemorySynchronously();
    ToyboxDisplacementTextureResource->FreeMainMemory();

    resources::CTextureResource* LightBulbTextureResource = resources::LoadTexture(LightBulbTextureFilePath);
    LightBulbTextureResource->LoadDataToMainMemorySynchronously();
    LightBulbTextureResource->LoadDataToVideoMemorySynchronously();
    LightBulbTextureResource->FreeMainMemory();

    FString BackpackMeshFilePath{ "assets/meshes/BackpackMesh.asset" };
    FString BackpackMeshDiffuseTexturePath{ "assets/textures/Backpack_TextureDiffuse.asset" };
    FString BackpackMeshNormalTexturePath{ "assets/textures/Backpack_TextureNormal.asset" };
    FString BackpackMeshSpecularTexturePath{ "assets/textures/Backpack_TextureSpecular.asset" };

    resources::CMeshResource* BackpackMeshResource = resources::LoadMesh(BackpackMeshFilePath);
    BackpackMeshResource->LoadDataToMainMemorySynchronously();
    BackpackMeshResource->LoadDataToVideoMemorySynchronously();
    BackpackMeshResource->FreeMainMemory();

    resources::CTextureResource* BackpackMeshDiffuseTexture = resources::LoadTexture(BackpackMeshDiffuseTexturePath);
    BackpackMeshDiffuseTexture->LoadDataToMainMemorySynchronously();
    BackpackMeshDiffuseTexture->LoadDataToVideoMemorySynchronously();
    BackpackMeshDiffuseTexture->FreeMainMemory();

    resources::CTextureResource* BackpackMeshNormalTexture = resources::LoadTexture(BackpackMeshNormalTexturePath);
    BackpackMeshNormalTexture->LoadDataToMainMemorySynchronously();
    BackpackMeshNormalTexture->LoadDataToVideoMemorySynchronously();
    BackpackMeshNormalTexture->FreeMainMemory();

    resources::CTextureResource* BackpackMeshSpecularTexture = resources::LoadTexture(BackpackMeshSpecularTexturePath);
    BackpackMeshSpecularTexture->LoadDataToMainMemorySynchronously();
    BackpackMeshSpecularTexture->LoadDataToVideoMemorySynchronously();
    BackpackMeshSpecularTexture->FreeMainMemory();

    scene::CBlinnPhongMapsMaterial* BackpackMaterial =
      new scene::CBlinnPhongMapsMaterial{ gpu::GShadersManager.GetShaderByName("BlinnPhongMaps") };
    BackpackMaterial->Shininess = 32;
    BackpackMaterial->DiffuseMap = BackpackMeshDiffuseTexture->TextureHandle;
    BackpackMaterial->NormalMap = BackpackMeshNormalTexture->TextureHandle;
    BackpackMaterial->SpecularMap = BackpackMeshSpecularTexture->TextureHandle;

    auto* BackpackStaticMesh = new scene::CStaticMesh{
        CopyToString("Backpack"), nullptr, BackpackMeshResource->VAO, BackpackMaterial, scene::EStaticMeshType::STATIONARY
    };

    FString SkyboxRightTextureFilePath{ "assets/textures/SkyboxRight.asset" };
    FString SkyboxLeftTextureFilePath{ "assets/textures/SkyboxLeft.asset" };
    FString SkyboxTopTextureFilePath{ "assets/textures/SkyboxTop.asset" };
    FString SkyboxBottomTextureFilePath{ "assets/textures/SkyboxBottom.asset" };
    FString SkyboxFrontTextureFilePath{ "assets/textures/SkyboxFront.asset" };
    FString SkyboxBackTextureFilePath{ "assets/textures/SkyboxBack.asset" };

    resources::CTextureResource* SkyboxRightTexture = resources::LoadTexture(SkyboxRightTextureFilePath);
    SkyboxRightTexture->LoadDataToMainMemorySynchronously();

    resources::CTextureResource* SkyboxLeftTexture = resources::LoadTexture(SkyboxLeftTextureFilePath);
    SkyboxLeftTexture->LoadDataToMainMemorySynchronously();

    resources::CTextureResource* SkyboxTopTexture = resources::LoadTexture(SkyboxTopTextureFilePath);
    SkyboxTopTexture->LoadDataToMainMemorySynchronously();

    resources::CTextureResource* SkyboxBottomTexture = resources::LoadTexture(SkyboxBottomTextureFilePath);
    SkyboxBottomTexture->LoadDataToMainMemorySynchronously();

    resources::CTextureResource* SkyboxFrontTexture = resources::LoadTexture(SkyboxFrontTextureFilePath);
    SkyboxFrontTexture->LoadDataToMainMemorySynchronously();

    resources::CTextureResource* SkyboxBackTexture = resources::LoadTexture(SkyboxBackTextureFilePath);
    SkyboxBackTexture->LoadDataToMainMemorySynchronously();

    const void* SkyboxFacesData[6]{ SkyboxRightTexture->TextureData, SkyboxLeftTexture->TextureData,
                                    SkyboxTopTexture->TextureData,   SkyboxBottomTexture->TextureData,
                                    SkyboxFrontTexture->TextureData, SkyboxBackTexture->TextureData };

    scene::CSkybox* Skybox =
      scene::CreateSkybox(SkyboxFacesData, SkyboxLeftTexture->Width, SkyboxLeftTexture->Height, FString{ "Skybox" });

    SkyboxRightTexture->FreeMainMemory();
    SkyboxLeftTexture->FreeMainMemory();
    SkyboxTopTexture->FreeMainMemory();
    SkyboxBottomTexture->FreeMainMemory();
    SkyboxFrontTexture->FreeMainMemory();
    SkyboxBackTexture->FreeMainMemory();

    // Configure the camera
    GSceneEditorState.PerspectiveCamera.AspectRatio =
      GSceneEditorState.ImSceneWindow->Size.x / GSceneEditorState.ImSceneWindow->Size.y;
    GSceneEditorState.PerspectiveCamera.Position = { 0, 0, 3 };
    GSceneEditorState.PerspectiveCamera.Yaw = -90.f;
    GSceneEditorState.PerspectiveCamera.UpdateCameraVectors();
    GSceneEditorState.PerspectiveCamera.Right = 1920;
    GSceneEditorState.PerspectiveCamera.Top = 1080;
    GSceneEditorState.CurrentCamera = &GSceneEditorState.PerspectiveCamera;

    // Setup the renderer
    scene::CForwardRenderer Renderer{ 32, 4 };
    Renderer.AmbientStrength = 0.05;
    Renderer.NumSamplesPCF = 20;
    Renderer.ResultResolution = { 1920, 1080 };
    Renderer.LightBulbTexture = LightBulbTextureResource->TextureHandle;
    Renderer.Setup();

    scene::CBlinnPhongMapsMaterial woodMaterial{ gpu::GShadersManager.GetShaderByName("BlinnPhongMaps") };
    woodMaterial.Shininess = 32;
    woodMaterial.DiffuseMap = WoodDiffuseTextureResource->TextureHandle;
    woodMaterial.SpecularColor = glm::vec3{ 0.5 };
    woodMaterial.NormalMap = nullptr;
    woodMaterial.SpecularMap = nullptr;

    scene::CBlinnPhongMapsMaterial brickMaterial{ gpu::GShadersManager.GetShaderByName("BlinnPhongMaps") };
    brickMaterial.Shininess = 32;
    brickMaterial.DiffuseMap = BrickDiffuseTextureResource->TextureHandle;
    brickMaterial.SpecularMap = nullptr;
    brickMaterial.DisplacementMap = nullptr;
    brickMaterial.NormalMap = BrickNormalTextureResource->TextureHandle;
    brickMaterial.SpecularColor = glm::vec3{ 0.2 };

    scene::CBlinnPhongMapsMaterial toyboxMaterial{ gpu::GShadersManager.GetShaderByName("BlinnPhongMaps") };
    toyboxMaterial.Shininess = 32;
    toyboxMaterial.DiffuseMap = WoodDiffuseTextureResource->TextureHandle;
    toyboxMaterial.SpecularMap = nullptr;
    toyboxMaterial.NormalMap = ToyboxNormalTextureResource->TextureHandle;
    toyboxMaterial.DisplacementMap = ToyboxDisplacementTextureResource->TextureHandle;
    toyboxMaterial.SpecularColor = glm::vec3{ 0.2 };

    scene::CBlinnPhongMaterial flatBlinnPhongMaterial{ gpu::GShadersManager.GetShaderByName("BlinnPhong") };
    flatBlinnPhongMaterial.DiffuseColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.SpecularColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.Shininess = 32;

    scene::CStaticMesh woodenFloor{
        FDString{ "woodenFloor" }, nullptr, QuadVAO, &woodMaterial, scene::EStaticMeshType::STATIONARY
    };
    woodenFloor.Transform.Scale = glm::vec3{ 25.0 };
    woodenFloor.Transform.Rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3{ 1.0, 0.0, 0.0 });
    woodenFloor.Transform.Translation = glm::vec3{ 0, -0.5, 0 };

    scene::CStaticMesh cube{ FDString{ "Cube" }, nullptr, UnitCubeVAO, &brickMaterial, scene::EStaticMeshType::STATIONARY };
    cube.Transform.Translation = { 4.0, -3.5, 0.0 };
    cube.Transform.Scale = glm::vec3{ 0.5 };

    scene::CStaticMesh cube1{
        FDString{ "Cube1" }, nullptr, UnitCubeVAO, &flatBlinnPhongMaterial, scene::EStaticMeshType::STATIONARY
    };
    cube1.Transform.Translation = { 2.0, 3.0, 1.0 };
    cube1.Transform.Scale = glm::vec3{ 0.75 };

    scene::CStaticMesh cube2{ FDString{ "Cube2" }, nullptr, UnitCubeVAO, &brickMaterial, scene::EStaticMeshType::STATIONARY };
    cube2.Transform.Translation = { -1.5, 2.0, -3.0 };
    cube2.Transform.Scale = glm::vec3{ 0.75 };

    scene::CStaticMesh cube3{ FDString{ "Cube3" }, nullptr, UnitCubeVAO, &toyboxMaterial, scene::EStaticMeshType::STATIONARY };
    cube3.Transform.Translation = { -1.5, 1.0, 1.5 };
    cube3.Transform.Scale = glm::vec3{ 0.75 };

    scene::CStaticMesh gigaCube{
        FDString{ "Gigacube" }, nullptr, UnitCubeVAO, &woodMaterial, scene::EStaticMeshType::STATIONARY
    };
    gigaCube.Transform.Translation = glm::vec3{ 0 };
    gigaCube.Transform.Scale = glm::vec3{ 10 };
    gigaCube.Transform.Rotation = glm::quat{ 0, 0, 0, 0 };
    gigaCube.SetReverseNormals(true);

    // scene::CStaticMesh* backPackRenderable = scene::CreateBlinnPhongRenderable(FString{ LUCID_TEXT("MyMesh") }, backPackMesh,
    // BlinnPhongMapsShader); backPackRenderable->Transform.Scale = { 0.25, 0.25, 0.25 };
    // backPackRenderable->Transform.Translation = { 0.0, 0.0, 0.0 };

    scene::FlatMaterial flatWhiteMaterial{ gpu::GShadersManager.GetShaderByName("Flat") };
    flatWhiteMaterial.Color = { 1.0, 1.0, 1.0, 1.0 };

    scene::FlatMaterial flatRedMaterial{ gpu::GShadersManager.GetShaderByName("Flat") };
    flatRedMaterial.Color = { 1.0, 0.0, 0.0, 1.0 };

    scene::FlatMaterial flatGreenMaterial{ gpu::GShadersManager.GetShaderByName("Flat") };
    flatGreenMaterial.Color = { 0.0, 1.0, 0.0, 1.0 };

    scene::FlatMaterial flatBlueMaterial{ gpu::GShadersManager.GetShaderByName("Flat") };
    flatBlueMaterial.Color = { 0.0, 0.0, 1.0, 1.0 };

    scene::CDirectionalLight* DirectionalLight = Renderer.CreateDirectionalLight(FDString{ "DirectionalLight" }, nullptr, true);
    DirectionalLight->Direction = glm::normalize(glm::vec3{ 0.5, -1, 1 });
    DirectionalLight->Transform.Translation = { -2.0f, 4.0f, -1.0f };
    DirectionalLight->Color = glm::vec3{ 1.0, 1.0, 1.0 };

    scene::CSpotLight* RedSpotLight = Renderer.CreateSpotLight(FDString{ "RedSpotLight" }, nullptr, true);
    RedSpotLight->Transform.Translation = cube2.Transform.Translation + glm::vec3{ 0, 2, -1.5 };
    RedSpotLight->Direction = glm::normalize(cube2.Transform.Translation - RedSpotLight->Transform.Translation);
    RedSpotLight->Color = { 1, 0, 0 };
    RedSpotLight->Constant = 1;
    RedSpotLight->Linear = 0.09;
    RedSpotLight->Quadratic = 0.032;
    RedSpotLight->InnerCutOffRad = glm::radians(30.0);
    RedSpotLight->OuterCutOffRad = glm::radians(35.0);

    scene::CSpotLight* GreenSpotLight = Renderer.CreateSpotLight(FDString{ "GreenSpotLight" }, nullptr, true);
    GreenSpotLight->Transform.Translation = cube.Transform.Translation + glm::vec3(0, 2, -2.5);
    GreenSpotLight->Direction = glm::normalize(cube.Transform.Translation - GreenSpotLight->Transform.Translation);
    GreenSpotLight->Color = { 0, 1, 0 };
    GreenSpotLight->Constant = 1;
    GreenSpotLight->Linear = 0.09;
    GreenSpotLight->Quadratic = 0.032;
    GreenSpotLight->InnerCutOffRad = glm::radians(30.0);
    GreenSpotLight->OuterCutOffRad = glm::radians(35.0);

    scene::CSpotLight* BlueSpotLight = Renderer.CreateSpotLight(FDString{ "BlueSpotLight" }, nullptr, true);
    BlueSpotLight->Transform.Translation = { 0, 5, 0 };
    BlueSpotLight->Direction = { 0, -1, 0 };
    BlueSpotLight->Color = { 0, 0, 1 };
    BlueSpotLight->Constant = 1;
    BlueSpotLight->Linear = 0.09;
    BlueSpotLight->Quadratic = 0.032;
    BlueSpotLight->InnerCutOffRad = glm::radians(30.0);
    BlueSpotLight->OuterCutOffRad = glm::radians(35.0);
    BlueSpotLight->LightUp = { -1, 0, 0 };

    scene::CStaticMesh shadowCastingLightCube{
        FDString{ "ShadowCastingLightCube" }, nullptr, UnitCubeVAO, &flatWhiteMaterial, scene::EStaticMeshType::STATIONARY
    };
    shadowCastingLightCube.Transform.Translation = DirectionalLight->Transform.Translation;

    scene::CPointLight* RedPointLight = Renderer.CreatePointLight(FDString{ "RedSpotLight" }, nullptr, true);
    RedPointLight->Transform.Translation = { 0, 0, 1.5 };
    RedPointLight->Color = { 1, 0, 0 };
    RedPointLight->Constant = 1;
    RedPointLight->Linear = 0.007;
    RedPointLight->Quadratic = 0.017;

    scene::CWorld DemoWorld;
    DemoWorld.Init();
    DemoWorld.AddStaticMesh(&cube);
    DemoWorld.AddStaticMesh(&cube1);
    DemoWorld.AddStaticMesh(&cube2);
    DemoWorld.AddStaticMesh(&cube3);
    DemoWorld.AddStaticMesh(&gigaCube);
    DemoWorld.AddStaticMesh(BackpackStaticMesh);
    DemoWorld.SetSkybox(Skybox);
    // sceneToRender.StaticGeometry.Add(&woodenFloor);

    DemoWorld.AddLight(RedSpotLight);
    DemoWorld.AddLight(GreenSpotLight);
    DemoWorld.AddLight(BlueSpotLight);
    DemoWorld.AddLight(RedPointLight);

    gpu::SetClearColor(BlackColor);

    GSceneEditorState.World = &DemoWorld;
    GSceneEditorState.Renderer = &Renderer;

    bool IsRunning = true;

    real now = platform::GetCurrentTimeSeconds();
    real last = 0;
    real dt = 0;

    scene::FRenderView RenderView;
    RenderView.Camera = GSceneEditorState.CurrentCamera; // @TODO is this needed???
    RenderView.Viewport = { 0, 0, Renderer.ResultResolution.x, Renderer.ResultResolution.y };

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
        Renderer.Render(DemoWorld.MakeRenderScene(GSceneEditorState.CurrentCamera), &RenderView);

        GSceneEditorState.Window->ImgUiStartNewFrame();
        {
            UISetupDockspace();
            ImGui::ShowDemoWindow();
            UIDrawSceneWindow();
            UIDrawAssetBrowserWindow();
            UIDrawSceneHierarchyWindow();
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

    gpu::Shutdown();
    return 0;
}

void InitializeSceneEditor()
{
    // Create the window for the editor
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
            ImGuiID AssetBrowserWindowDockId;
            ImGuiID SceneHierarchyDockId;
            ImGuiID SceneWindowDockId;
            SceneWindowDockId =
              ImGui::DockBuilderSplitNode(GSceneEditorState.MainDockId, ImGuiDir_Left, 0.75f, nullptr, &SceneHierarchyDockId);
            SceneWindowDockId =
              ImGui::DockBuilderSplitNode(SceneWindowDockId, ImGuiDir_Up, 0.7f, nullptr, &AssetBrowserWindowDockId);

            // Attach windows to dockspaces
            ImGui::DockBuilderDockWindow(SCENE_VIEWPORT, SceneWindowDockId);
            ImGui::DockBuilderDockWindow(ASSET_BROWSER, AssetBrowserWindowDockId);
            ImGui::DockBuilderDockWindow(SCENE_HIERARCHY, SceneHierarchyDockId);

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
    GSceneEditorState.Window->ImgUiDrawFrame();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void HandleCameraMovement(const float& DeltaTime)
{
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

    if (IsMouseButtonPressed(RIGHT) && !GSceneEditorState.CurrentlyDraggedActor)
    {
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
        // Drop the current actor if we're outside
        if (GSceneEditorState.CurrentlyDraggedActor)
        {
            GSceneEditorState.CurrentlyDraggedActor = nullptr;
        }
        return;
    }

    // Check if the scene window is not obscured by some other widget
    if (ImGui::GetFocusID() != GSceneEditorState.ImSceneWindow->ID && ImGui::GetFocusID() != GSceneEditorState.SceneDockId)
    {
        return;
    }

    if (GSceneEditorState.CurrentlyDraggedActor)
    {
        if (IsMouseButtonPressed(LEFT))
        {
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
            ActorPosView.x = -MouseRayView.x * GSceneEditorState.DistanceToCurrentlyDraggedActor;
            ActorPosView.y = -MouseRayView.y * GSceneEditorState.DistanceToCurrentlyDraggedActor;

            // Update actor's position
            GSceneEditorState.CurrentlyDraggedActor->Transform.Translation =
              (glm::inverse(GSceneEditorState.CurrentCamera->GetViewMatrix()) * ActorPosView);
        }
        else
        {
            GSceneEditorState.CurrentlyDraggedActor = nullptr;
        }
    }
    else if (IsMouseButtonPressed(LEFT))
    {
        // Check if've hit something based on the hit map generated by the renderer
        const scene::FHitMap& CachedHitMap = GSceneEditorState.Renderer->GetCachedHitMap();

        // Adjust mouse postion based on hitmap texture size
        const float RatioX = MousePosRelative.x / GSceneEditorState.SceneWindowWidth;
        const float RatioY = MousePosRelative.y / GSceneEditorState.SceneWindowHeight;

        const glm::vec2 AdjustedMousePos = { ((float)CachedHitMap.Width) * RatioX, ((float)CachedHitMap.Height) * RatioY };

        scene::IActor* ClickedActor = GSceneEditorState.World->GetActorById(CachedHitMap.GetIdAtMousePositon(AdjustedMousePos));

        // Ignore skyboxes
        if (ClickedActor != nullptr && ClickedActor->GetActorType() != scene::EActorType::SKYBOX)
        {
            // Remember the actor that we hit and how far from the camera it was on the z axis
            glm::vec4 ActorPosView =
              GSceneEditorState.CurrentCamera->GetViewMatrix() * glm::vec4{ ClickedActor->Transform.Translation, 1 };
            GSceneEditorState.DistanceToCurrentlyDraggedActor = ActorPosView.z;
            GSceneEditorState.CurrentlyDraggedActor = ClickedActor;
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
        GSceneEditorState.Renderer->GetResultFramebuffer()->ImGuiDrawToImage(
          { GSceneEditorState.SceneWindowWidth, GSceneEditorState.SceneWindowHeight });
    }
    ImGui::PopStyleVar(1);
    ImGui::End();
}

void UIDrawAssetBrowserWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
    ImGui::Begin(ASSET_BROWSER, nullptr, WindowFlags);
    {
        // Menu bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Asset import
                if (ImGui::BeginMenu("Import asset"))
                {
                    if (ImGui::MenuItem("Mesh"))
                    {
                        GSceneEditorState.FileDialog.SetTitle("Select a mesh file");
                        GSceneEditorState.FileDialog.SetTypeFilters({ ".obj", ".fbx" });
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
    }
    ImGui::End();
}

void UIDrawSceneHierarchyWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    ImGui::Begin(SCENE_HIERARCHY, nullptr, WindowFlags);
    {
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
}

void ImportMesh(const std::filesystem::path& SelectedFilePath)
{
}