#include "platform/window.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/init.hpp"
#include "misc/basic_shapes.hpp"
#include "devices/gpu/texture.hpp"
#include "platform/fs.hpp"
#include "platform/input.hpp"
#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/gpu.hpp"
#include "stdio.h"
#include "devices/gpu/vao.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "scene/camera.hpp"
#include "devices/gpu/viewport.hpp"
#include "scene/fwd_blinn_phong_renderer.hpp"
#include "scene/blinn_phong_material.hpp"
#include "scene/render_scene.hpp"
#include "scene/renderable.hpp"
#include "scene/lights.hpp"
#include "stb_init.hpp"
#include "resources/texture.hpp"
#include "resources/mesh.hpp"
#include "GL/glew.h"
#include <time.h>
#include "devices/gpu/cubemap.hpp"
#include "scene/flat_material.hpp"
#include "glm/gtc/quaternion.hpp"
#include "platform/util.hpp"

using namespace lucid;

int main(int argc, char** argv)
{
    srand(time(NULL));
    InitSTB();

    if (gpu::Init({}) < 0)
    {
        puts("Failed to init GPU");
        return -1;
    }

    // create window

    platform::Window* window = platform::CreateWindow({ "Lucid", 900, 420, 1280, 720, true });
    misc::InitBasicShapes();
    resources::InitTextures();

    // Create a framebuffer and it's attachments

    gpu::Texture* colorAttachment = gpu::CreateEmpty2DTexture(window->GetWidth(), window->GetHeight(),
                                                              gpu::TextureDataType::FLOAT, gpu::TextureFormat::RGBA, 0);
    gpu::FramebufferAttachment* renderbuffer =
      gpu::CreateRenderbuffer(gpu::RenderbufferFormat::DEPTH24_STENCIL8, { window->GetWidth(), window->GetHeight() });
    gpu::Framebuffer* testFramebuffer = gpu::CreateFramebuffer();
    gpu::Framebuffer* shadowMapFramebuffer = gpu::CreateFramebuffer();

    shadowMapFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
    shadowMapFramebuffer->DisableReadWriteBuffers();

    renderbuffer->Bind();
    colorAttachment->Bind();
    colorAttachment->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    colorAttachment->SetMagFilter(gpu::MagTextureFilter::LINEAR);
    testFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);

    testFramebuffer->SetupColorAttachment(0, colorAttachment);
    testFramebuffer->SetupDepthStencilAttachment(renderbuffer);
    testFramebuffer->IsComplete();

    gpu::BindDefaultFramebuffer(gpu::FramebufferBindMode::READ_WRITE);

    // Load textures uesd in the demo scene

    // resources::MeshResource* backPackMesh = resources::AssimpLoadMesh("assets\\models\\backpack\\", "backpack.obj");

    resources::TextureResource* brickWallDiffuseMapResource =
      resources::LoadJPEG("assets/textures/brickwall.jpg", true, gpu::TextureDataType::UNSIGNED_BYTE, true);
    resources::TextureResource* brickWallNormalMapResource =
      resources::LoadJPEG("assets/textures/brickwall_normal.jpg", false, gpu::TextureDataType::UNSIGNED_BYTE, true);

    resources::TextureResource* woodDiffuseMapResource =
      resources::LoadJPEG("assets/textures/wood.png", true, gpu::TextureDataType::UNSIGNED_BYTE, true);

    resources::TextureResource* blankTextureResource =
      resources::LoadPNG("assets/textures/blank.png", true, gpu::TextureDataType::UNSIGNED_BYTE, true);

    brickWallDiffuseMapResource->FreeMainMemory();
    brickWallNormalMapResource->FreeMainMemory();
    woodDiffuseMapResource->FreeMainMemory();
    blankTextureResource->FreeMainMemory();

    // backPackMesh->FreeMainMemory();

    auto brickWallDiffuseMap = brickWallDiffuseMapResource->TextureHandle;
    auto brickWallNormalMap = brickWallNormalMapResource->TextureHandle;
    auto woodDiffuseMap = woodDiffuseMapResource->TextureHandle;
    auto blankTextureMap = blankTextureResource->TextureHandle;

    // auto backpackVao = backPackMesh->VAO;

    window->Prepare();
    window->Show();

    // Load and compile demo shaders

    gpu::Shader* blinnPhongShader =
      gpu::CompileShaderProgram("BlinnPhong", { platform::ReadFile("shaders/glsl/fwd_blinn_phong.vert", true) },
                                { platform::ReadFile("shaders/glsl/fwd_blinn_phong.frag", true) }, "");

    gpu::Shader* blinnPhongMapsShader =
      gpu::CompileShaderProgram("BlinnPhongMaps", { platform::ReadFile("shaders/glsl/fwd_blinn_phong_maps.vert", true) },
                                { platform::ReadFile("shaders/glsl/fwd_blinn_phong_maps.frag", true) }, "", true);

    gpu::Shader* skyboxShader = gpu::CompileShaderProgram("Skybox", { platform::ReadFile("shaders/glsl/skybox.vert", true) },
                                                          { platform::ReadFile("shaders/glsl/skybox.frag", true) }, "");

    gpu::Shader* shadowMapShader =
      gpu::CompileShaderProgram("ShadowMapper", { platform::ReadFile("shaders/glsl/shadow_map.vert", true) },
                                { platform::ReadFile("shaders/glsl/empty.frag", true) }, "");

    gpu::Shader* shadowCubemapShader =
      gpu::CompileShaderProgram("ShadowCubeMapper", { platform::ReadFile("shaders/glsl/shadow_cubemap.vert", true) },
                                { platform::ReadFile("shaders/glsl/shadow_cubemap.frag", true) },
                                { platform::ReadFile("shaders/glsl/shadow_cubemap.geom", true) });

    gpu::Shader* flatShader = gpu::CompileShaderProgram("flatShader", { platform::ReadFile("shaders/glsl/flat.vert", true) },
                                                        { platform::ReadFile("shaders/glsl/flat.frag", true) }, "");

    // Prepare the scene

    gpu::Viewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    scene::Camera perspectiveCamera{ scene::CameraMode::PERSPECTIVE };
    perspectiveCamera.AspectRatio = window->GetAspectRatio();
    perspectiveCamera.Position = { 0, 0, 3 };
    perspectiveCamera.Yaw = -90.f;
    perspectiveCamera.UpdateCameraVectors();

    scene::ForwardBlinnPhongRenderer renderer{ 32, blinnPhongMapsShader, skyboxShader };
    renderer.AmbientStrength = 0.05;
    renderer.NumSamplesPCF = 20;

    scene::RenderTarget renderTarget;
    renderTarget.Camera = &perspectiveCamera;
    renderTarget.Framebuffer = testFramebuffer;
    renderTarget.Viewport = windowViewport;

    scene::BlinnPhongMapsMaterial woodMaterial;
    woodMaterial.Shininess = 32;
    woodMaterial.DiffuseMap = woodDiffuseMap;
    woodMaterial.SpecularColor = glm::vec3{ 0.5 };
    woodMaterial.NormalMap = nullptr;
    woodMaterial.SpecularMap = nullptr;

    scene::BlinnPhongMapsMaterial brickMaterial;
    brickMaterial.Shininess = 32;
    brickMaterial.DiffuseMap = brickWallDiffuseMap;
    brickMaterial.SpecularMap = nullptr;
    brickMaterial.NormalMap = brickWallNormalMap;
    brickMaterial.SpecularColor = glm::vec3{ 0.2 };

    scene::BlinnPhongMaterial flatBlinnPhongMaterial;
    flatBlinnPhongMaterial.DiffuseColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.SpecularColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.Shininess = 32;
    flatBlinnPhongMaterial.SetCustomShader(blinnPhongShader);

    scene::Renderable woodenFloor{ "woodenFloor" };
    woodenFloor.Material = &woodMaterial;
    woodenFloor.VertexArray = misc::QuadVertexArray;
    woodenFloor.Type = scene::RenderableType::STATIC;
    woodenFloor.Transform.Scale = glm::vec3{ 25.0 };
    woodenFloor.Transform.Rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3{ 1.0, 0.0, 0.0 });
    woodenFloor.Transform.Translation = glm::vec3{ 0, -0.5, 0 };

    scene::Renderable cube{ "Cube" };
    cube.Material = &brickMaterial;
    cube.VertexArray = misc::CubeVertexArray;
    cube.Type = scene::RenderableType::STATIC;
    cube.Transform.Translation = { 4.0, -3.5, 0.0 };
    cube.Transform.Scale = glm::vec3{ 0.5 };

    scene::Renderable cube1{ "Cube1", cube };
    cube1.Transform.Translation = { 2.0, 3.0, 1.0 };
    cube1.Transform.Scale = glm::vec3{ 0.75 };
    cube1.Material = &flatBlinnPhongMaterial;

    scene::Renderable cube2{ "Cube2", cube };
    cube2.Transform.Translation = { -1.5, 2.0, -3.0 };
    cube2.Transform.Scale = glm::vec3{ 0.75 };
    cube2.Transform.Rotation = glm::angleAxis(glm::radians(60.0f), glm::normalize(glm::vec3{ 1.0, 0.0, 1.0 }));

    scene::Renderable cube3{ "Cube3", cube };
    cube3.Transform.Translation = { -1.5, 1.0, 1.5 };
    cube3.Transform.Scale = glm::vec3{ 0.5 };
    cube3.Transform.Rotation = glm::angleAxis(glm::radians(60.0f), glm::normalize(glm::vec3{ 1.0, 0.0, 1.0 }));

    scene::Renderable gigaCube{ "Gigacube", cube };
    gigaCube.Transform.Translation = glm::vec3{ 0 };
    gigaCube.Transform.Scale = glm::vec3{ 10 };
    gigaCube.Transform.Rotation = glm::quat{ 0, 0, 0, 0 };
    gigaCube.Material = &woodMaterial;
    gigaCube.bReverseNormals = true;

    // scene::Renderable* backPackRenderable = scene::CreateBlinnPhongRenderable("MyMesh", backPackMesh);
    // backPackRenderable->Transform.Scale = { 0.25, 0.25, 0.25 };
    // backPackRenderable->Transform.Translation = { 0.0, 0.0, 2.0 };

    scene::FlatMaterial flatWhiteMaterial;
    flatWhiteMaterial.Color = { 1.0, 1.0, 1.0, 1.0 };
    flatWhiteMaterial.SetCustomShader(flatShader);

    scene::FlatMaterial flatRedMaterial;
    flatRedMaterial.Color = { 1.0, 0.0, 0.0, 1.0 };
    flatRedMaterial.SetCustomShader(flatShader);

    scene::FlatMaterial flatGreenMaterial;
    flatGreenMaterial.Color = { 0.0, 1.0, 0.0, 1.0 };
    flatGreenMaterial.SetCustomShader(flatShader);

    scene::FlatMaterial flatBlueMaterial;
    flatBlueMaterial.Color = { 0.0, 0.0, 1.0, 1.0 };
    flatBlueMaterial.SetCustomShader(flatShader);

    scene::DirectionalLight shadowCastingLight = scene::CreateDirectionalLight(true, { 1024, 1024 });
    shadowCastingLight.Direction = glm::normalize(glm::vec3{ 0.5, -1, 1 });
    shadowCastingLight.Position = { -2.0f, 4.0f, -1.0f };
    shadowCastingLight.Color = glm::vec3{ 1.0, 1.0, 1.0 };

    scene::SpotLight redLight = scene::CreateSpotLight(true, { 1024, 1024 });
    redLight.Position = { 5, 3.4, 0 };
    redLight.Direction = glm::normalize(glm::vec3{ -1, -1, 0 });
    redLight.Color = { 1, 0, 0 };
    redLight.Constant = 1;
    redLight.Linear = 0.09;
    redLight.Quadratic = 0.032;
    redLight.InnerCutOffRad = glm::radians(30.0);
    redLight.OuterCutOffRad = glm::radians(35.0);

    scene::Renderable redLightCube{ "RedLightCube", cube };
    redLightCube.Transform.Scale = glm::vec3{ 0.2 };
    redLightCube.Material = &flatRedMaterial;
    redLightCube.Transform.Translation = redLight.Position;

    scene::SpotLight greenLight = scene::CreateSpotLight(true, { 1024, 1024 });
    greenLight.Position = { -5, 3.5, 0 };
    greenLight.Direction = glm::normalize(glm::vec3{ 1, -1, 0 });
    greenLight.Color = { 0, 1, 0 };
    greenLight.Constant = 1;
    greenLight.Linear = 0.09;
    greenLight.Quadratic = 0.032;
    greenLight.InnerCutOffRad = glm::radians(30.0);
    greenLight.OuterCutOffRad = glm::radians(35.0);

    scene::Renderable greenLightCube{ "GreenLightCube", redLightCube };
    greenLightCube.Transform.Translation = greenLight.Position;
    greenLightCube.Material = &flatGreenMaterial;

    scene::SpotLight blueLight = scene::CreateSpotLight(true, { 1024, 1024 });
    blueLight.Position = { 0, 5, 0 };
    blueLight.Direction = { 0, -1, 0 };
    blueLight.Color = { 0, 0, 1 };
    blueLight.Constant = 1;
    blueLight.Linear = 0.09;
    blueLight.Quadratic = 0.032;
    blueLight.InnerCutOffRad = glm::radians(30.0);
    blueLight.OuterCutOffRad = glm::radians(35.0);
    blueLight.LightUp = { -1, 0, 0 };

    scene::Renderable blueLightCube{ "BlueLightCube", redLightCube };
    blueLightCube.Transform.Translation = blueLight.Position;
    blueLightCube.Material = &flatBlueMaterial;

    scene::Renderable shadowCastingLightCube{ "ShadowCastingLightCube", redLightCube };
    shadowCastingLightCube.Transform.Translation = shadowCastingLight.Position;
    shadowCastingLightCube.Material = &flatWhiteMaterial;

    scene::PointLight redPointLight = scene::CreatePointLight(true, { 1024, 1024 });
    redPointLight.Position = { 0, 0, 0 };
    redPointLight.Color = { 1, 0, 0 };
    redPointLight.Constant = 1;
    redPointLight.Linear = 0.007;
    redPointLight.Quadratic = 0.017;

    scene::Renderable redPointLightCube{ "RedPointLightCube", redLightCube };
    redPointLightCube.Transform.Translation = redPointLight.Position;
    redPointLightCube.Material = &flatRedMaterial;

    scene::RenderScene sceneToRender;
    sceneToRender.StaticGeometry.Add(&cube);
    sceneToRender.StaticGeometry.Add(&cube1);
    sceneToRender.StaticGeometry.Add(&cube2);
    sceneToRender.StaticGeometry.Add(&cube3);
    sceneToRender.StaticGeometry.Add(&gigaCube);
    // sceneToRender.StaticGeometry.Add(backPackRenderable);
    // sceneToRender.StaticGeometry.Add(&woodenFloor);

    sceneToRender.Lights.Add(&redLight);
    sceneToRender.Lights.Add(&greenLight);
    sceneToRender.Lights.Add(&blueLight);
    // sceneToRender.Lights.Add(&shadowCastingLight);
    sceneToRender.Lights.Add(&redPointLight);

    // sceneToRender.StaticGeometry.Add(&shadowCastingLightCube);
    sceneToRender.StaticGeometry.Add(&redLightCube);
    sceneToRender.StaticGeometry.Add(&greenLightCube);
    sceneToRender.StaticGeometry.Add(&blueLightCube);
    sceneToRender.StaticGeometry.Add(&redPointLightCube);

    const char* skyboxFacesPaths[] = { "assets/skybox/right.jpg",  "assets/skybox/left.jpg",  "assets/skybox/top.jpg",
                                       "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg" };
    scene::Skybox skybox = scene::CreateSkybox(skyboxFacesPaths);
    sceneToRender.SceneSkybox = &skybox;

    gpu::SetClearColor(BlackColor);

    bool isRunning = true;
    float rotation = 0;

    real now = platform::GetCurrentTimeSeconds();
    real last = 0;
    real dt = 0;

    while (isRunning)
    {
        last = now;
        now = platform::GetCurrentTimeSeconds();
        dt += now - last;
        ReadEvents(window);

        while (dt > 0)
        {
            dt -= platform::SimulationStep;
            if (WasKeyPressed(SDLK_ESCAPE))
            {
                isRunning = false;
                break;
            }

            if (IsKeyPressed(SDLK_w))
            {
                perspectiveCamera.MoveForward(platform::SimulationStep);
            }

            if (IsKeyPressed(SDLK_s))
            {
                perspectiveCamera.MoveBackward(platform::SimulationStep);
            }

            if (IsKeyPressed(SDLK_a))
            {
                perspectiveCamera.MoveLeft(platform::SimulationStep);
            }

            if (IsKeyPressed(SDLK_d))
            {
                perspectiveCamera.MoveRight(platform::SimulationStep);
            }

            auto mousePos = GetMousePostion();
            if (mousePos.MouseMoved)
            {
                perspectiveCamera.AddRotation(-mousePos.DeltaX, mousePos.DeltaY);
            }

            if (IsKeyPressed(SDLK_r))
            {
                rotation += 0.5f;
                cube.Transform.Rotation = glm::angleAxis(glm::radians(rotation), glm::normalize(glm::vec3{ 0.0, 1.0, 0.0 }));
            }

            redPointLight.Position.z = sin(now * 0.5) * 3.0;
            redPointLightCube.Transform.Translation = redPointLight.Position;
        }

        gpu::DisableSRGBFramebuffer();
        shadowCastingLight.UpdateLightSpaceMatrix();
        shadowCastingLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowMapShader, true, true);

        redLight.UpdateLightSpaceMatrix();
        redLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowMapShader, true, true);

        greenLight.UpdateLightSpaceMatrix();
        greenLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowMapShader, true, true);

        blueLight.UpdateLightSpaceMatrix();
        blueLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowMapShader, true, true);

        redPointLight.UpdateLightSpaceMatrix();
        redPointLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowCubemapShader, true, true);

        // Render to off-screen framebuffer
        renderTarget.Framebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
        renderer.Render(&sceneToRender, &renderTarget);

        // Blit the off-screen frame buffer to the window framebuffer
        gpu::BindDefaultFramebuffer(gpu::FramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
        gpu::EnableSRGBFramebuffer();
        gpu::BlitFramebuffer(
          renderTarget.Framebuffer, nullptr, true, false, false,
          { 0, 0, renderTarget.Framebuffer->GetColorAttachmentSize().x, renderTarget.Framebuffer->GetColorAttachmentSize().y },
          { 0, 0, window->GetWidth(), window->GetHeight() });
        window->Swap();
    }

    delete blinnPhongShader;

    window->Destroy();

    return 0;
}