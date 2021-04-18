#include <devices/gpu/texture_enums.hpp>
#include <scene/material.hpp>

#include "engine_init.hpp"
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
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"


using namespace lucid;

constexpr char EDITOR_WINDOW[] = "Lucid Editor";
constexpr char DOCKSPACE_WINDOW[] = "Dockspace";
constexpr char SCENE_VIEWPORT[] = "Scene";
constexpr char ASSET_BROWSER[] = "Asset browser";
constexpr char MAIN_DOCKSPACE[] = "MainDockSpace";

struct FSceneEditorState
{
    platform::CWindow* Window;
    ImGuiContext* ImGuiContext;
    ImGuiWindow*  ImSceneWindow;

    ImGuiID MainDockId = 0;
    ImGuiID SceneDockId = 0;
    
    u16     EditorWindowWidth = 1920;
    u16     EditorWindowHeight = 1080;
    float   EditorWindowAspectRatio = 1280.f / 720.f;

    float SceneWindowWidth;
    float SceneWindowHeight;
    
    /** User interface stuff */
    bool bIsAnyUIWidgetActive = false;
    bool bIsAnyUIWidgetFocused = false;

    scene::CWorld* World = nullptr;
    scene::CRenderer* Renderer = nullptr; // @TODO this should be an engine variable
} GSceneEditorState;

void InitializeSceneEditor();
void UIUpdateState();

void UISetupDockspace();
void UIDrawSceneWindow();
void UIDrawAssetBrowserWindow();

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

    // Prepare the scene
    gpu::FViewport WindowViewport { 0, 0, GSceneEditorState.EditorWindowWidth, GSceneEditorState.EditorWindowHeight };

    scene::CCamera PerspectiveCamera{ scene::ECameraMode::PERSPECTIVE };
    PerspectiveCamera.AspectRatio = GSceneEditorState.ImSceneWindow->Size.x / GSceneEditorState.ImSceneWindow->Size.y;
    PerspectiveCamera.Position = { 0, 0, 3 };
    PerspectiveCamera.Yaw = -90.f;
    PerspectiveCamera.UpdateCameraVectors();
    PerspectiveCamera.Right = 1920;
    PerspectiveCamera.Top = 1080;

    scene::CForwardRenderer Renderer{ 32, 4 };
    Renderer.AmbientStrength = 0.05;
    Renderer.NumSamplesPCF = 20;
    Renderer.FramebufferSize = { 1920, 1080 };
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
    
    bool isRunning = true;
    float rotation = 0;

    real now = platform::GetCurrentTimeSeconds();
    real last = 0;
    real dt = 0;

    scene::FRenderView RenderView;
    RenderView.Camera = &PerspectiveCamera;
    RenderView.Viewport = WindowViewport;

    scene::IActor* CurrentlyDraggedActor = nullptr;
    float DistanceToActor = 0;
    while (isRunning)
    {
        platform::Update();
        last = now;
        now = platform::GetCurrentTimeSeconds();
        dt += now - last;
        ReadEvents(GSceneEditorState.Window);

        while (dt > platform::SimulationStep)
        {
            dt -= platform::SimulationStep;
            if (WasKeyPressed(SDLK_ESCAPE))
            {
                isRunning = false;
                break;
            }

            if (IsKeyPressed(SDLK_w))
            {
                PerspectiveCamera.MoveForward(platform::SimulationStep);
            }

            if (IsKeyPressed(SDLK_s))
            {
                PerspectiveCamera.MoveBackward(platform::SimulationStep);
            }

            if (IsKeyPressed(SDLK_a))
            {
                PerspectiveCamera.MoveLeft(platform::SimulationStep);
            }

            if (IsKeyPressed(SDLK_d))
            {
                PerspectiveCamera.MoveRight(platform::SimulationStep);
            }

            auto mousePos = GetMousePostion();
            if (IsMouseButtonPressed(RIGHT) && !CurrentlyDraggedActor)
            {
                PerspectiveCamera.AddRotation(-mousePos.DeltaX * 17.5 * dt, mousePos.DeltaY * 17.5 * dt);
                CurrentlyDraggedActor = nullptr;
            }
            else if (CurrentlyDraggedActor)
            {
                if (IsMouseButtonPressed(LEFT))
                {
                    glm::vec4 ActorPosView = PerspectiveCamera.GetViewMatrix() * glm::vec4{ CurrentlyDraggedActor->Transform.Translation, 1 };

                    const glm::vec2 MouseRayNDC = 2.f * glm::vec2{
                        GetMousePostion().X / GSceneEditorState.SceneWindowWidth,
                        1 - (GetMousePostion().Y / GSceneEditorState.SceneWindowHeight) } - 1.f;

                    const glm::vec4 MouseRayClip { MouseRayNDC, -1, 1 };
                    const glm::vec4 MouseRayView = glm::inverse(PerspectiveCamera.GetProjectionMatrix()) * MouseRayClip;

                    ActorPosView.x = -MouseRayView.x * DistanceToActor;
                    ActorPosView.y = -MouseRayView.y * DistanceToActor;

                    CurrentlyDraggedActor->Transform.Translation =
                      (glm::inverse(PerspectiveCamera.GetViewMatrix()) * ActorPosView);
                }
                else
                {
                    CurrentlyDraggedActor = nullptr;
                }
            }
            else if (IsMouseButtonPressed(LEFT) &&
                     (!GSceneEditorState.bIsAnyUIWidgetActive && !GSceneEditorState.bIsAnyUIWidgetFocused))
            {
                const scene::FHitMap& CachedHitMap = Renderer.GetCachedHitMap();
                scene::IActor* ClickedActor = DemoWorld.GetActorById(CachedHitMap.GetIdAtMousePositon(GetMousePostion()));

                if (ClickedActor != nullptr && ClickedActor->GetActorType() != scene::EActorType::SKYBOX)
                {
                    glm::vec4 ActorPosView =
                      PerspectiveCamera.GetViewMatrix() * glm::vec4{ ClickedActor->Transform.Translation, 1 };
                    DistanceToActor = ActorPosView.z;
                    CurrentlyDraggedActor = ClickedActor;
                }
            }

            if (IsKeyPressed(SDLK_r))
            {
                rotation += 0.5f;
                cube.Transform.Rotation = glm::angleAxis(glm::radians(rotation), glm::normalize(glm::vec3{ 0.0, 1.0, 0.0 }));
            }

            // RedPointLight->Transform.Translation.z = sin(now * 0.5) * 3.0;
        }

        // Render the scene to off-screen framebuffer
        GSceneEditorState.Window->Prepare();
        Renderer.Render(DemoWorld.MakeRenderScene(&PerspectiveCamera), &RenderView);

        GSceneEditorState.Window->ImgUiStartNewFrame();
        {
            UISetupDockspace();
            ImGui::ShowDemoWindow();
            UIDrawSceneWindow();
            UIDrawAssetBrowserWindow();
            UIUpdateState();
        }

        GSceneEditorState.Window->Clear();
        GSceneEditorState.Window->ImgUiDrawFrame();
        GSceneEditorState.Window->Swap();   

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        
        // Blit the off-screen frame buffer to the window framebuffer
        // GSceneEditorState.EditorWindow->GetFramebuffer()->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        // gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));
        // gpu::EnableSRGBFramebuffer();
        // gpu::BlitFramebuffer(Renderer.GetResultFramebuffer(),
                             // GSceneEditorState.EditorWindow->GetFramebuffer(),
                             // true,
                             // false,
                             // false,
                             // { 0, 0, Renderer.FramebufferSize.x, Renderer.FramebufferSize.y },
                             // { 0, 0, GSceneEditorState.EditorWindow->GetWidth(), GSceneEditorState.EditorWindow->GetHeight() });

    }

    gpu::Shutdown();
    return 0;
}

void InitializeSceneEditor()
{
    // Create a hidden window so we can have a GL Context to render to
    platform::FWindowDefiniton HiddenWindow;
    HiddenWindow.title = "Lucid Editor";
    HiddenWindow.x = 100;
    HiddenWindow.y = 100;
    HiddenWindow.width = 1920;
    HiddenWindow.height = 1080;
    HiddenWindow.sRGBFramebuffer = true;
    HiddenWindow.bHidden = false;
    
    GSceneEditorState.Window = platform::CreateNewWindow(HiddenWindow);
    GSceneEditorState.Window->Prepare();
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    
    GSceneEditorState.ImGuiContext = ImGui::CreateContext();

    // Configure ImGui
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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
            ImGui::DockBuilderSetNodeSize(GSceneEditorState.MainDockId, ImVec2((float) GSceneEditorState.EditorWindowWidth, (float) GSceneEditorState.EditorWindowHeight));

            // Split the dockspace
            ImGuiID AssetBrowserWindowDockId;
            ImGuiID SceneWindowDockId = ImGui::DockBuilderSplitNode(GSceneEditorState.MainDockId, ImGuiDir_Left, 0.8f, nullptr, &AssetBrowserWindowDockId);

            // Attach windows to dockspaces
            ImGui::DockBuilderDockWindow(SCENE_VIEWPORT, SceneWindowDockId);
            ImGui::DockBuilderDockWindow(ASSET_BROWSER, AssetBrowserWindowDockId);

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

void UIUpdateState()
{
    // Update flags based on current UI state
    GSceneEditorState.bIsAnyUIWidgetActive = ImGui::IsAnyItemHovered();
    GSceneEditorState.bIsAnyUIWidgetFocused = ImGui::IsAnyItemFocused();

    // GSceneEditorState.EditorWindow->ImgUiStartNewFrame();

    // ImGui::Begin();
    // ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

    // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // ImGui::End();
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
    ImGui::PopStyleVar(2);
    
    ImGui::DockSpace(GSceneEditorState.MainDockId, ImVec2(0.0f, 0.0f));
    ImGui::End();
}

void UIDrawSceneWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin(SCENE_VIEWPORT, nullptr, WindowFlags);
    {
        GSceneEditorState.SceneWindowWidth = ImGui::GetContentRegionAvail().x; 
        GSceneEditorState.SceneWindowHeight = ImGui::GetContentRegionAvail().y; 
        GSceneEditorState.Renderer->GetResultFramebuffer()->ImGuiDrawToImage({GSceneEditorState.SceneWindowWidth, GSceneEditorState.SceneWindowHeight});
    }
    ImGui::End();
}

void UIDrawAssetBrowserWindow()
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;;

    ImGui::Begin(ASSET_BROWSER, nullptr, WindowFlags);
    {
    }
    ImGui::End();
    
}