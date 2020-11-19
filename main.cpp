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

    platform::Window* window = platform::CreateWindow({ "Lucid", 900, 420, 800, 600, true });
    misc::InitBasicShapes();
    resources::InitTextures();

    // Create a framebuffer and it's attachments

    gpu::Texture* colorAttachment = gpu::CreateEmpty2DTexture(window->GetWidth(), window->GetHeight(),
                                                              gpu::TextureDataType::FLOAT, gpu::TextureFormat::RGBA, 0);
    gpu::FramebufferAttachment* renderbuffer = gpu::CreateRenderbuffer(gpu::RenderbufferFormat::DEPTH24_STENCIL8, { 800, 600 });
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

    resources::TextureResource* brickWallDiffuseMapResource = resources::LoadJPEG("assets/textures/brick-diffuse-map.jpg", true, gpu::TextureDataType::UNSIGNED_BYTE);
    resources::TextureResource* brickWallNormalMapResource = resources::LoadJPEG("assets/textures/brick-normal-map.png", true, gpu::TextureDataType::UNSIGNED_BYTE);
    
    resources::TextureResource* woodDiffuseMapResource = resources::LoadJPEG("assets/textures/wood.png", true, gpu::TextureDataType::UNSIGNED_BYTE);

    resources::TextureResource* blankTextureResource = resources::LoadPNG("assets/textures/blank.png", true, gpu::TextureDataType::UNSIGNED_BYTE);

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

    gpu::Shader* blinnPhongShader = gpu::CompileShaderProgram({ platform::ReadFile("shaders/glsl/fwd_blinn_phong.vert", true) },
                                                              { platform::ReadFile("shaders/glsl/fwd_blinn_phong.frag", true) });

    gpu::Shader* blinnPhongMapsShader =
      gpu::CompileShaderProgram({ platform::ReadFile("shaders/glsl/fwd_blinn_phong_maps.vert", true) },
                                { platform::ReadFile("shaders/glsl/fwd_blinn_phong_maps.frag", true) });

    gpu::Shader* skyboxShader =
      gpu::CompileShaderProgram({ platform::ReadFile("shaders/glsl/skybox.vert", true) },
                                { platform::ReadFile("shaders/glsl/skybox.frag", true) });

    gpu::Shader* shadowMapShader =
      gpu::CompileShaderProgram({ platform::ReadFile("shaders/glsl/shadow_map.vert", true) },
                                { platform::ReadFile("shaders/glsl/empty.frag", true) });


    // Prepare the scene

    gpu::Viewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    scene::Camera perspectiveCamera{ scene::CameraMode::PERSPECTIVE, { 0, 0, 2.5 } };
    perspectiveCamera.AspectRatio = window->GetAspectRatio();

    scene::ForwardBlinnPhongRenderer renderer{ 32, blinnPhongMapsShader, skyboxShader };
    renderer.SetAmbientStrength(0.05);

    scene::RenderTarget renderTarget;
    renderTarget.Camera = &perspectiveCamera;
    renderTarget.Framebuffer = testFramebuffer;
    renderTarget.Viewport = windowViewport;

    scene::BlinnPhongMaterial flatMaterial;
    flatMaterial.Shininess = 32;
    flatMaterial.DiffuseColor = glm::vec3(1.0, 0.0, 0.0);
    flatMaterial.SpecularColor = glm::vec3(1.0, 0.0, 0.0);
    flatMaterial.SetCustomShader(blinnPhongShader);

    scene::BlinnPhongMaterial lightMaterial;
    flatMaterial.Shininess = 32;
    flatMaterial.DiffuseColor = glm::vec3(1.0, 1.0, 1.0);
    flatMaterial.SpecularColor = glm::vec3(1.0, 1.0, 1.0);
    flatMaterial.SetCustomShader(blinnPhongShader);

    scene::BlinnPhongMapsMaterial brickMaterial;
    brickMaterial.Shininess = 32;
    brickMaterial.DiffuseMap = brickWallDiffuseMap;
    brickMaterial.SpecularMap = blankTextureMap;
    brickMaterial.NormalMap = brickWallNormalMap;

    scene::BlinnPhongMapsMaterial woodMaterial;
    woodMaterial.Shininess = 32;
    woodMaterial.DiffuseMap = woodDiffuseMap;

    scene::Renderable brickWall { "BrickWall" };
    brickWall.Transform.Translation = { -2, 0, -4 };
    brickWall.Material = &brickMaterial;
    brickWall.VertexArray = misc::QuadVertexArray;
    brickWall.Type = scene::RenderableType::STATIC;

    scene::Renderable woodenFloor { "woodenFloor ", brickWall };
    woodenFloor.Transform.Scale =  { 10.0, 10.0, 10.0 };
    woodenFloor.Transform.Translation =  { 0, -2, -7.5 };
    woodenFloor.Transform.Rotation.x = 1;
    woodenFloor.Transform.Rotation.y = 0;
    woodenFloor.Transform.Rotation.z = 0;
    woodenFloor.Transform.Rotation.w = glm::radians(-90.f);
    woodenFloor.Material = &woodMaterial;

    scene::Renderable cube{ "Cube" };
    cube.Transform.Translation = { 2, 0, -2 };
    cube.Material = &flatMaterial;
    cube.VertexArray = misc::CubeVertexArray;
    cube.Type = scene::RenderableType::STATIC;
    cube.Transform.Scale = glm::vec3(0.5);
    // scene::Renderable* backPackRenderable = scene::CreateBlinnPhongRenderable("MyMesh", backPackMesh);

    scene::Renderable cube1{ "Cube1", cube };
    scene::Renderable cube2{ "Cube2", cube };
    scene::Renderable cube3{ "Cube3", cube };
    scene::Renderable cube4{ "Cube4", cube };

    cube1.Transform.Translation = { -1.5, 0, -5 };
    cube2.Transform.Translation = { 0, 0, -8 };
    cube3.Transform.Translation = { -2, 0, -11 };
    cube4.Transform.Translation = { 2, 0, -15 };
    cube.Transform.Rotation = glm::quat(glm::vec3(0.0, 45.0, 0.0));

    scene::DirectionalLight whiteDirLight = scene::CreateDirectionalLight(true, {1024, 1024});
    whiteDirLight.Direction = { 0, 0, 0 };
    whiteDirLight.Position = { -2.0f, 4.0f, -1.0f };

    scene::PointLight redLight;
    redLight.Position = { 2, 0, 0 };
    redLight.Color = { 1, 0, 0 };
    redLight.Constant = 1;
    redLight.Linear = 0.09;
    redLight.Quadratic = 0.032;

    scene::SpotLight greenLight;
    greenLight.Position = { 0, 0, 1 };
    greenLight.Color = { 0, 1, 0 };
    greenLight.Constant = 1;
    greenLight.Linear = 0.09;
    greenLight.Quadratic = 0.032;
    greenLight.Direction = { 0, 0, -1 };
    greenLight.InnerCutOffRad = glm::radians(30.0);
    greenLight.OuterCutOffRad = glm::radians(35.0);

    scene::PointLight blueLight;
    blueLight.Position = { -2, 0, 0 };
    blueLight.Color = { 0, 0, 1 };
    blueLight.Constant = 1;
    blueLight.Linear = 0.09;
    blueLight.Quadratic = 0.032;

    scene::DirectionalLight whiteLight;
    whiteLight.Direction = { 0, -1, -1 };
    whiteLight.Color = { 1, 1, 1 };

    scene::RenderScene sceneToRender;
    sceneToRender.StaticGeometry.Add(&cube);
    sceneToRender.StaticGeometry.Add(&cube1);
    sceneToRender.StaticGeometry.Add(&cube2);
    sceneToRender.StaticGeometry.Add(&cube3);
    sceneToRender.StaticGeometry.Add(&cube4);
    sceneToRender.StaticGeometry.Add(&brickWall);
    sceneToRender.StaticGeometry.Add(&woodenFloor);
    // sceneToRender.Renderables.Add(backPackRenderable);
    sceneToRender.Lights.Add(&redLight);
    sceneToRender.Lights.Add(&greenLight);
    sceneToRender.Lights.Add(&blueLight);
    // sceneToRender.Lights.Add(&whiteLight);

    const char* skyboxFacesPaths[] = { "assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg", "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg" };
    sceneToRender.SceneSkybox = scene::CreateSkybox(skyboxFacesPaths);

    gpu::SetClearColor(BlackColor);
    
    int currentRotation = 0;
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

        if (IsKeyPressed(SDLK_r))
        {
            currentRotation += 1;
            brickWall.Transform.Rotation.w = glm::radians((float)(currentRotation % 360));
        }

        if (IsMouseButtonPressed(MouseButton::LEFT))
        {
            auto mousePos = GetMousePostion();

            if (mousePos.MouseMoved)
            {
                perspectiveCamera.AddRotation(-mousePos.DeltaX, mousePos.DeltaY);
            }
        }

        gpu::DisableSRGBFramebuffer();
        whiteDirLight.GenerateShadowMap(&sceneToRender, shadowMapFramebuffer, shadowMapShader, true, true);

        // Render to off-screen framebuffer

        gpu::SetViewport(renderTarget.Viewport);
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