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

#include "resources/texture.hpp"
#include "resources/mesh.hpp"

#include "glm/gtc/quaternion.hpp"


using namespace lucid;

#include "imgui.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char** argv)
{
    FEngineConfig EngineConfig;
    EngineConfig.bHotReloadShaders = true;

    lucid::InitEngine(EngineConfig);
    resources::InitTextures();

    // create window
    platform::CWindow* window = platform::CreateNewWindow({"Lucid", 100, 100, 1280, 720, true});
    window->Prepare();
    window->Show();

    gpu::CVertexArray* UnitCubeVAO = misc::CreateCubeVAO();
    gpu::CVertexArray* QuadVAO = misc::CreateQuadVAO();


    // Load and compile demo shaders
    gpu::CShader* BlinnPhongShader =
        gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("BlinnPhong")},
                                           FString{LUCID_TEXT("shaders/glsl/fwd_blinn_phong.vert")},
                                           FString{LUCID_TEXT("shaders/glsl/fwd_blinn_phong.frag")},
                                           EMPTY_STRING);
    gpu::CShader* BlinnPhongMapsShader =
        gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("BlinnPhongMaps")},
                                           FString{LUCID_TEXT("shaders/glsl/fwd_blinn_phong_maps.vert")},
                                           FString{LUCID_TEXT("shaders/glsl/fwd_blinn_phong_maps.frag")},
                                           EMPTY_STRING);
    gpu::CShader* SkyboxShader = gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("Skybox")},
                                                                    FString{LUCID_TEXT("shaders/glsl/skybox.vert")},
                                                                    FString{LUCID_TEXT("shaders/glsl/skybox.frag")},
                                                                    EMPTY_STRING);
    gpu::CShader* ShadowMapShader = gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("ShadowMap")},
                                                                       FString{
                                                                           LUCID_TEXT("shaders/glsl/shadow_map.vert")
                                                                       },
                                                                       FString{LUCID_TEXT("shaders/glsl/empty.frag")},
                                                                       EMPTY_STRING);
    gpu::CShader* ShadowCubemapShader =
        gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("CubeShadowMap")},
                                           FString{LUCID_TEXT("shaders/glsl/shadow_cubemap.vert")},
                                           FString{LUCID_TEXT("shaders/glsl/shadow_cubemap.frag")},
                                           FString{LUCID_TEXT("shaders/glsl/shadow_cubemap.geom")});
    
    gpu::CShader* FlatShader = gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("FlatShadowMap")},
                                                                  FString{LUCID_TEXT("shaders/glsl/flat.vert")},
                                                                  FString{LUCID_TEXT("shaders/glsl/flat.frag")},
                                                                  EMPTY_STRING);
    gpu::CShader* ForwardPrepassShader =
        gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("ForwardPrepass")},
                                           FString{LUCID_TEXT("shaders/glsl/forward_prepass.vert")},
                                           FString{LUCID_TEXT("shaders/glsl/forward_prepass.frag")},
                                           EMPTY_STRING);
    gpu::CShader* SSAOShader = gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("SSAO")},
                                                                  FString{LUCID_TEXT("shaders/glsl/ssao.vert")},
                                                                  FString{LUCID_TEXT("shaders/glsl/ssao.frag")},
                                                                  EMPTY_STRING);
    gpu::CShader* SimpleBlurShader = gpu::GShadersManager.CompileShader(FString{LUCID_TEXT("Simple blur")},
                                                                        FString{
                                                                            LUCID_TEXT("shaders/glsl/simple_blur.vert")
                                                                        },
                                                                        FString{
                                                                            LUCID_TEXT("shaders/glsl/simple_blur.frag")
                                                                        },
                                                                        EMPTY_STRING);
    
    gpu::CShader* HitmapShader = gpu::GShadersManager.CompileShader(FString{"HitMapShader"},
                                                                    FString{"shaders/glsl/hit_map.vert"}, FString{
                                                                        "shaders/glsl/hit_map.frag"
                                                                    }, EMPTY_STRING);
    gpu::CShader* BillboardHitmapShader = gpu::GShadersManager.CompileShader(
        FString{"HitMapShader"}, FString{"shaders/glsl/billboard_hitmap.vert"}, FString{"shaders/glsl/hit_map.frag"},
        EMPTY_STRING);
    gpu::CShader* BillboardShader = gpu::GShadersManager.CompileShader(FString{"BillboardShader"},
                                                                       FString{"shaders/glsl/billboard.vert"}, FString{
                                                                           "shaders/glsl/billboard.frag"
                                                                       }, EMPTY_STRING);


    // Load textures and meshes used in the demo scene

    FString BrickDiffuseTextureFilePath         { "assets/textures/BrickDiffuse.asset" };
    FString BrickNormalTextureFilePath          { "assets/textures/BrickNormal.asset" };
    FString WoodDiffuseTextureFilePath          { "assets/textures/WoodDiffuse.asset" };
    FString BlankTextureFilePath                { "assets/textures/Blank.asset" };
    FString ToyboxNormalTextureFilePath         { "assets/textures/ToyboxNormal.asset" };
    FString ToyboxDisplacementTextureFilePath   { "assets/textures/ToyboxDisplacement.asset" };
    FString LightBulbTextureFilePath            { "assets/textures/LightBulb.asset" };

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

    FString BackpackMeshFilePath                { "assets/meshes/BackpackMesh.asset" };
    FString BackpackMeshDiffuseTexturePath      { "assets/textures/Backpack_TextureDiffuse.asset" };
    FString BackpackMeshNormalTexturePath       { "assets/textures/Backpack_TextureNormal.asset" };
    FString BackpackMeshSpecularTexturePath     { "assets/textures/Backpack_TextureSpecular.asset" };
    
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

    scene::CBlinnPhongMapsMaterial* BackpackMaterial = new scene::CBlinnPhongMapsMaterial { BlinnPhongMapsShader };
    BackpackMaterial->Shininess     = 32;
    BackpackMaterial->DiffuseMap    = BackpackMeshDiffuseTexture->TextureHandle;
    BackpackMaterial->NormalMap     = BackpackMeshNormalTexture->TextureHandle;
    BackpackMaterial->SpecularMap   = BackpackMeshSpecularTexture->TextureHandle;

    auto* BackpackStaticMesh = new scene::CStaticMesh { CopyToString("Backpack"), nullptr, BackpackMeshResource->VAO, BackpackMaterial, scene::EStaticMeshType::STATIONARY };
    
    FString SkyboxRightTextureFilePath  { "assets/textures/SkyboxRight.asset" };
    FString SkyboxLeftTextureFilePath   { "assets/textures/SkyboxLeft.asset" };
    FString SkyboxTopTextureFilePath    { "assets/textures/SkyboxTop.asset" };
    FString SkyboxBottomTextureFilePath { "assets/textures/SkyboxBottom.asset" };
    FString SkyboxFrontTextureFilePath  { "assets/textures/SkyboxFront.asset" };
    FString SkyboxBackTextureFilePath   { "assets/textures/SkyboxBack.asset" };

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

    const void* SkyboxFacesData[6] {
        SkyboxRightTexture->TextureData,
        SkyboxLeftTexture->TextureData,
        SkyboxTopTexture->TextureData,
        SkyboxBottomTexture->TextureData,
        SkyboxFrontTexture->TextureData,
        SkyboxBackTexture->TextureData
    };
    
    scene::CSkybox* Skybox = scene::CreateSkybox(SkyboxFacesData, SkyboxLeftTexture->Width, SkyboxLeftTexture->Height, FString{ "Skybox" });

    SkyboxRightTexture->FreeMainMemory();
    SkyboxLeftTexture->FreeMainMemory();
    SkyboxTopTexture->FreeMainMemory();
    SkyboxBottomTexture->FreeMainMemory();
    SkyboxFrontTexture->FreeMainMemory();
    SkyboxBackTexture->FreeMainMemory();

    // Prepare the scene
    gpu::FViewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    
    scene::CCamera PerspectiveCamera{ scene::ECameraMode::PERSPECTIVE };
    PerspectiveCamera.AspectRatio = window->GetAspectRatio();
    PerspectiveCamera.Position = { 0, 0, 3 };
    PerspectiveCamera.Yaw = -90.f;
    PerspectiveCamera.UpdateCameraVectors();
    PerspectiveCamera.Right = window->GetWidth();
    PerspectiveCamera.Top = window->GetHeight();
    scene::ForwardRenderer Renderer{
        32,           64,           ShadowMapShader, ShadowCubemapShader, ForwardPrepassShader, SSAOShader, SimpleBlurShader,
        SkyboxShader, BillboardShader, FlatShader
    };
    Renderer.HitMapShader = HitmapShader;
    Renderer.BillboardHitMapShader = BillboardHitmapShader;
    Renderer.AmbientStrength = 0.05;
    Renderer.NumSamplesPCF = 20;
    Renderer.FramebufferSize = { window->GetWidth(), window->GetHeight() };
    Renderer.LightBulbTexture = LightBulbTextureResource->TextureHandle;
    Renderer.Setup();
    
    scene::CBlinnPhongMapsMaterial woodMaterial{ BlinnPhongMapsShader };
    woodMaterial.Shininess = 32;
    woodMaterial.DiffuseMap = WoodDiffuseTextureResource->TextureHandle;
    woodMaterial.SpecularColor = glm::vec3{ 0.5 };
    woodMaterial.NormalMap = nullptr;
    woodMaterial.SpecularMap = nullptr;
    
    scene::CBlinnPhongMapsMaterial brickMaterial{ BlinnPhongMapsShader };
    brickMaterial.Shininess = 32;
    brickMaterial.DiffuseMap = BrickDiffuseTextureResource->TextureHandle;
    brickMaterial.SpecularMap = nullptr;
    brickMaterial.DisplacementMap = nullptr;
    brickMaterial.NormalMap = BrickNormalTextureResource->TextureHandle;
    brickMaterial.SpecularColor = glm::vec3{ 0.2 };
    
    scene::CBlinnPhongMapsMaterial toyboxMaterial{ BlinnPhongMapsShader };
    toyboxMaterial.Shininess = 32;
    toyboxMaterial.DiffuseMap = WoodDiffuseTextureResource->TextureHandle;
    toyboxMaterial.SpecularMap = nullptr;
    toyboxMaterial.NormalMap = ToyboxNormalTextureResource->TextureHandle;
    toyboxMaterial.DisplacementMap = ToyboxDisplacementTextureResource->TextureHandle;
    toyboxMaterial.SpecularColor = glm::vec3{ 0.2 };
    
    scene::CBlinnPhongMaterial flatBlinnPhongMaterial{ BlinnPhongShader };
    flatBlinnPhongMaterial.DiffuseColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.SpecularColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.Shininess = 32;
    
    scene::CStaticMesh woodenFloor{ FDString{ "woodenFloor" },         nullptr, QuadVAO,
                                    &woodMaterial, scene::EStaticMeshType::STATIONARY };
    woodenFloor.Transform.Scale = glm::vec3{ 25.0 };
    woodenFloor.Transform.Rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3{ 1.0, 0.0, 0.0 });
    woodenFloor.Transform.Translation = glm::vec3{ 0, -0.5, 0 };
    
    scene::CStaticMesh cube{ FDString{ "Cube" }, nullptr, UnitCubeVAO, &brickMaterial, scene::EStaticMeshType::STATIONARY };
    cube.Transform.Translation = { 4.0, -3.5, 0.0 };
    cube.Transform.Scale = glm::vec3{ 0.5 };
    
    scene::CStaticMesh cube1{ FDString{ "Cube1" },     nullptr,
                              UnitCubeVAO, &flatBlinnPhongMaterial, scene::EStaticMeshType::STATIONARY };
    cube1.Transform.Translation = { 2.0, 3.0, 1.0 };
    cube1.Transform.Scale = glm::vec3{ 0.75 };
    
    scene::CStaticMesh cube2{ FDString{ "Cube2" }, nullptr, UnitCubeVAO, &brickMaterial, scene::EStaticMeshType::STATIONARY };
    cube2.Transform.Translation = { -1.5, 2.0, -3.0 };
    cube2.Transform.Scale = glm::vec3{ 0.75 };
    
    scene::CStaticMesh cube3{ FDString{ "Cube3" }, nullptr, UnitCubeVAO, &toyboxMaterial, scene::EStaticMeshType::STATIONARY };
    cube3.Transform.Translation = { -1.5, 1.0, 1.5 };
    cube3.Transform.Scale = glm::vec3{ 0.75 };
    
    scene::CStaticMesh gigaCube{ FDString{ "Gigacube" }, nullptr,
                                 UnitCubeVAO, &woodMaterial,          scene::EStaticMeshType::STATIONARY };
    gigaCube.Transform.Translation = glm::vec3{ 0 };
    gigaCube.Transform.Scale = glm::vec3{ 10 };
    gigaCube.Transform.Rotation = glm::quat{ 0, 0, 0, 0 };
    gigaCube.SetReverseNormals(true);
    
    // scene::CStaticMesh* backPackRenderable = scene::CreateBlinnPhongRenderable(FString{ LUCID_TEXT("MyMesh") }, backPackMesh,
    // BlinnPhongMapsShader); backPackRenderable->Transform.Scale = { 0.25, 0.25, 0.25 };
    // backPackRenderable->Transform.Translation = { 0.0, 0.0, 0.0 };
    
    scene::FlatMaterial flatWhiteMaterial{ FlatShader };
    flatWhiteMaterial.Color = { 1.0, 1.0, 1.0, 1.0 };
    
    scene::FlatMaterial flatRedMaterial{ FlatShader };
    flatRedMaterial.Color = { 1.0, 0.0, 0.0, 1.0 };
    
    scene::FlatMaterial flatGreenMaterial{ FlatShader };
    flatGreenMaterial.Color = { 0.0, 1.0, 0.0, 1.0 };
    
    scene::FlatMaterial flatBlueMaterial{ FlatShader };
    flatBlueMaterial.Color = { 0.0, 0.0, 1.0, 1.0 };
    
    scene::CDirectionalLight* DirectionalLight = Renderer.CreateDirectionalLight(FDString{"DirectionalLight"}, nullptr, true);
    DirectionalLight->Direction = glm::normalize(glm::vec3{ 0.5, -1, 1 });
    DirectionalLight->Transform.Translation = { -2.0f, 4.0f, -1.0f };
    DirectionalLight->Color = glm::vec3{ 1.0, 1.0, 1.0 };
    
    scene::CSpotLight* RedSpotLight = Renderer.CreateSpotLight(FDString{"RedSpotLight"}, nullptr, true);
    RedSpotLight->Transform.Translation = cube2.Transform.Translation + glm::vec3{ 0, 2, -1.5 };
    RedSpotLight->Direction = glm::normalize(cube2.Transform.Translation - RedSpotLight->Transform.Translation);
    RedSpotLight->Color = { 1, 0, 0 };
    RedSpotLight->Constant = 1;
    RedSpotLight->Linear = 0.09;
    RedSpotLight->Quadratic = 0.032;
    RedSpotLight->InnerCutOffRad = glm::radians(30.0);
    RedSpotLight->OuterCutOffRad = glm::radians(35.0);
    
    scene::CSpotLight* GreenSpotLight = Renderer.CreateSpotLight(FDString{"GreenSpotLight"}, nullptr,true);
    GreenSpotLight->Transform.Translation = cube.Transform.Translation + glm::vec3(0, 2, -2.5);
    GreenSpotLight->Direction = glm::normalize(cube.Transform.Translation - GreenSpotLight->Transform.Translation);
    GreenSpotLight->Color = { 0, 1, 0 };
    GreenSpotLight->Constant = 1;
    GreenSpotLight->Linear = 0.09;
    GreenSpotLight->Quadratic = 0.032;
    GreenSpotLight->InnerCutOffRad = glm::radians(30.0);
    GreenSpotLight->OuterCutOffRad = glm::radians(35.0);
    
    scene::CSpotLight* BlueSpotLight = Renderer.CreateSpotLight(FDString{"BlueSpotLight"}, nullptr,true);
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
    
    scene::CPointLight* RedPointLight = Renderer.CreatePointLight(FDString{"RedSpotLight"}, nullptr,true);
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
    
    bool isRunning = true;
    float rotation = 0;
    
    real now = platform::GetCurrentTimeSeconds();
    real last = 0;
    real dt = 0;
    
    scene::FRenderView RenderView;
    RenderView.Camera = &PerspectiveCamera;
    RenderView.Viewport = windowViewport;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    window->ImgUiSetup();
    
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    scene::IActor* CurrentlyDraggedActor = nullptr;
    float DistanceToActor = 0;
    while (isRunning)
    {
        platform::Update();
        last = now;
        now = platform::GetCurrentTimeSeconds();
        dt += now - last;
        ReadEvents(window);
    
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
                    glm::vec4 ActorPosView = PerspectiveCamera.GetViewMatrix() * glm::vec4 { CurrentlyDraggedActor->Transform.Translation, 1 };
    
                    const glm::vec2 MouseRayNDC = 2.f * glm::vec2 { GetMousePostion().X / (float) window->GetWidth(),  1 - (GetMousePostion().Y / (float) window->GetHeight()) } - 1.f;
                    const glm::vec4 MouseRayClip {  MouseRayNDC, -1, 1 };
                    const glm::vec4 MouseRayView = glm::inverse(PerspectiveCamera.GetProjectionMatrix()) * MouseRayClip;
    
                    ActorPosView.x = -MouseRayView.x * DistanceToActor;
                    ActorPosView.y = -MouseRayView.y * DistanceToActor;
                    
                    CurrentlyDraggedActor->Transform.Translation =  (glm::inverse(PerspectiveCamera.GetViewMatrix()) * ActorPosView);
                }
                else
                {
                    CurrentlyDraggedActor = nullptr;
                }
            }
            else if (IsMouseButtonPressed(LEFT))
            {
                const scene::FHitMap& CachedHitMap = Renderer.GetCachedHitMap();
                scene::IActor* ClickedActor = DemoWorld.GetActorById(CachedHitMap.GetIdAtMousePositon(GetMousePostion()));
    
                if (ClickedActor != nullptr && ClickedActor->GetActorType() != scene::EActorType::SKYBOX)
                {
                    glm::vec4 ActorPosView = PerspectiveCamera.GetViewMatrix() * glm::vec4 { ClickedActor->Transform.Translation, 1 };
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
    
        // Render to off-screen framebuffer
        Renderer.Render(DemoWorld.MakeRenderScene(&PerspectiveCamera), &RenderView);
    
        // Blit the off-screen frame buffer to the window framebuffer
        window->GetFramebuffer()->Bind(gpu::EFramebufferBindMode::READ_WRITE);
    gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));
        gpu::EnableSRGBFramebuffer();
        gpu::BlitFramebuffer(Renderer.GetResultFramebuffer(),
                             window->GetFramebuffer(),
                             true,
                             false,
                             false,
                             { 0, 0, Renderer.FramebufferSize.x, Renderer.FramebufferSize.y },
                             { 0, 0, window->GetWidth(), window->GetHeight() });
    
        // Img Ui stuff
        window->ImgUiStartNewFrame();
    
        {
            static float f = 0.0f;
            static int counter = 0;
    
            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
    
            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
            ImGui::Checkbox("Another Window", &show_another_window);
    
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
    
            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
    
            ImGui::Text(
              "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
    
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a
                                                                  // closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        
    window->ImgUiDrawFrame();
    window->Swap();
    }
    
    window->Destroy();
    gpu::Shutdown();

    return 0;
}
