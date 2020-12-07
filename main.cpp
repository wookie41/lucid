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
      resources::LoadJPEG("assets/textures/brick-diffuse-map.jpg", true, gpu::TextureDataType::UNSIGNED_BYTE, true);
    resources::TextureResource* brickWallNormalMapResource =
      resources::LoadJPEG("assets/textures/brick-normal-map.png", true, gpu::TextureDataType::UNSIGNED_BYTE, true);

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
                                { platform::ReadFile("shaders/glsl/fwd_blinn_phong.frag", true) });

    gpu::Shader* blinnPhongMapsShader =
      gpu::CompileShaderProgram("BlinnPhongMaps", { platform::ReadFile("shaders/glsl/fwd_blinn_phong_maps.vert", true) },
                                { platform::ReadFile("shaders/glsl/fwd_blinn_phong_maps.frag", true) }, true);

    gpu::Shader* skyboxShader = gpu::CompileShaderProgram("Skybox", { platform::ReadFile("shaders/glsl/skybox.vert", true) },
                                                          { platform::ReadFile("shaders/glsl/skybox.frag", true) });

    gpu::Shader* shadowMapShader =
      gpu::CompileShaderProgram("ShadowMapper", { platform::ReadFile("shaders/glsl/shadow_map.vert", true) },
                                { platform::ReadFile("shaders/glsl/empty.frag", true) });

    gpu::Shader* flatShader = gpu::CompileShaderProgram("flatShader", { platform::ReadFile("shaders/glsl/flat.vert", true) },
                                                        { platform::ReadFile("shaders/glsl/flat.frag", true) });

    // Prepare the scene

    gpu::Viewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    scene::Camera perspectiveCamera{ scene::CameraMode::PERSPECTIVE };
    perspectiveCamera.AspectRatio = window->GetAspectRatio();

    scene::ForwardBlinnPhongRenderer renderer{ 32, blinnPhongMapsShader, skyboxShader };
    renderer.SetAmbientStrength(0.075);

    scene::RenderTarget renderTarget;
    renderTarget.Camera = &perspectiveCamera;
    renderTarget.Framebuffer = testFramebuffer;
    renderTarget.Viewport = windowViewport;

    scene::BlinnPhongMapsMaterial woodMaterial;
    woodMaterial.Shininess = 32;
    woodMaterial.DiffuseMap = woodDiffuseMap;

    scene::BlinnPhongMaterial flatBlinnPhongMaterial;
    flatBlinnPhongMaterial.DiffuseColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.SpecularColor = glm::vec3{ 1 };
    flatBlinnPhongMaterial.Shininess = 32;
    flatBlinnPhongMaterial.SetCustomShader(blinnPhongShader);

    scene::Renderable woodenFloor{ "woodenFloor" };
    woodenFloor.Material = &woodMaterial;
    woodenFloor.VertexArray = misc::QuadVertexArray;
    woodenFloor.Type = scene::RenderableType::STATIC;

    scene::Renderable cube{ "Cube" };
    cube.Material = &woodMaterial;
    cube.VertexArray = misc::CubeVertexArray;
    cube.Type = scene::RenderableType::STATIC;
    cube.Transform.Translation = { 0.0, 1.5, 0.0 };
    cube.Transform.Scale = glm::vec3{ 0.5 };

    scene::Renderable cube1{ "Cube1", cube };
    cube1.Transform.Translation = { 2.0, 0.0, 1.0 };
    cube1.Transform.Scale = glm::vec3{ 0.5 };
    cube1.Material = &flatBlinnPhongMaterial;

    scene::Renderable cube2{ "Cube2", cube };
    glm::vec3 cubeRotAxis = glm::normalize(glm::vec3{ 1, 0, 1 });
    cube2.Transform.Translation = { -1.0f, 0.0f, 2.0 };
    cube2.Transform.Scale = glm::vec3{ 0.25 };
    cube2.Transform.Rotation.x = cubeRotAxis.x;
    cube2.Transform.Rotation.y = cubeRotAxis.y;
    cube2.Transform.Rotation.z = cubeRotAxis.z;
    cube2.Transform.Rotation.w = glm::radians(60.0);

    // scene::Renderable* backPackRenderable = scene::CreateBlinnPhongRenderable("MyMesh", backPackMesh);
    // backPackRenderable->Transform.Scale = { 0.25, 0.25, 0.25 };

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

    scene::DirectionalLight redDirLight = scene::CreateDirectionalLight(true, { 1024, 1024 });
    redDirLight.Direction = glm::normalize(glm::vec3{ 2.0f, -4.0f, 1.0f });
    redDirLight.Position = { -2.0f, 4.0f, -1.0f };
    redDirLight.Color = glm::vec3{ 0.2, 0.0, 0.0 };

    scene::Renderable redDirLightCube{ "redDirLightCube", cube };
    redDirLightCube.Material = &flatWhiteMaterial;
    redDirLightCube.Transform.Translation = redDirLight.Position;

    scene::SpotLight redLight;
    redLight.Position = { 2, 4, 0 };
    redLight.Direction = { -2, -4, 0 };
    redLight.Color = { 1, 0, 0 };
    redLight.Constant = 1;
    redLight.Linear = 0.09;
    redLight.Quadratic = 0.032;
    redLight.InnerCutOffRad = glm::radians(30.0);
    redLight.OuterCutOffRad = glm::radians(35.0);

    scene::Renderable redLightCube{ "RedLightCube", cube };
    redLightCube.Transform.Scale = glm::vec3{ 0.25 };
    redLightCube.Material = &flatRedMaterial;
    redLightCube.Transform.Translation = redLight.Position;

    scene::SpotLight greenLight = redLight;
    greenLight.Position = { -2, 4, 0 };
    greenLight.Direction = { 2, -4, 0 };
    greenLight.Color = { 0, 1, 0 };

    scene::Renderable greenLightCube{ "GreenLightCube", redLightCube };
    greenLightCube.Transform.Translation = greenLight.Position;
    greenLightCube.Material = &flatGreenMaterial;

    scene::PointLight blueLight = greenLight;
    greenLight.Position = { 0, 4, 0 };
    greenLight.Direction = { 0, -4, 0 };
    blueLight.Color = { 0, 0, 1 };

    scene::Renderable blueLightCube{ "BlueLightCube", redLightCube };
    blueLightCube.Transform.Translation = greenLight.Position;
    blueLightCube.Material = &flatBlueMaterial;

    scene::DirectionalLight whiteLight;
    whiteLight.Direction = { 0, -1, -1 };
    whiteLight.Color = { 1, 1, 1 };

    scene::RenderScene sceneToRender;
    sceneToRender.StaticGeometry.Add(&cube);
    sceneToRender.StaticGeometry.Add(&cube1);
    sceneToRender.StaticGeometry.Add(&cube2);

    sceneToRender.StaticGeometry.Add(&woodenFloor);

    // sceneToRender.Lights.Add(&redDirLight);
    sceneToRender.Lights.Add(&redLight);
    sceneToRender.Lights.Add(&greenLight);
    sceneToRender.Lights.Add(&blueLight);

    // sceneToRender.StaticGeometry.Add(&redDirLightCube);
    sceneToRender.StaticGeometry.Add(&redLightCube);
    sceneToRender.StaticGeometry.Add(&greenLightCube);
    sceneToRender.StaticGeometry.Add(&blueLightCube);

    const char* skyboxFacesPaths[] = { "assets/skybox/right.jpg",  "assets/skybox/left.jpg",  "assets/skybox/top.jpg",
                                       "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg" };
    scene::Skybox skybox = scene::CreateSkybox(skyboxFacesPaths);
    sceneToRender.SceneSkybox = &skybox;

    gpu::SetClearColor(BlackColor);

    bool isRunning = true;
    while (isRunning)
    {
        ReadEvents();

        if (WasKeyPressed(SDLK_ESCAPE))
        {
            isRunning = false;
            break;
        }

        if (IsKeyPressed(SDLK_w))
        {
            perspectiveCamera.MoveForward(1.f / 60.f);
        }

        if (IsKeyPressed(SDLK_s))
        {
            perspectiveCamera.MoveBackward(1.f / 60.f);
        }

        if (IsKeyPressed(SDLK_a))
        {
            perspectiveCamera.MoveLeft(1.f / 60.f);
        }

        if (IsKeyPressed(SDLK_d))
        {
            perspectiveCamera.MoveRight(1.f / 60.f);
        }
        auto mousePos = GetMousePostion();
        if (mousePos.MouseMoved)
        {
            perspectiveCamera.AddRotation(-mousePos.DeltaX, mousePos.DeltaY);
        }

        gpu::DisableSRGBFramebuffer();
        redDirLight.UpdateLightSpaceMatrix();
        redDirLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowMapShader, true, true);

        // Render to off-screen framebuffer
        renderTarget.Framebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
        gpu::DisableSRGBFramebuffer();
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