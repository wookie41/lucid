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
#include <time.h>
#include "resources/texture.hpp"
#include "resources/mesh.hpp"
#include "GL/glew.h"

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

    platform::Window* window = platform::CreateWindow({ "Lucid", 900, 420, 800, 600 });
    misc::InitBasicShapes();
    resources::InitTextures();

    gpu::Texture* colorAttachment =
      gpu::CreateEmpty2DTexture(window->GetWidth(), window->GetHeight(), gpu::TextureFormat::RGBA, 0);
    gpu::FramebufferAttachment* renderbuffer = gpu::CreateRenderbuffer(gpu::RenderbufferFormat::DEPTH24_STENCIL8, 800, 600);
    gpu::Framebuffer* testFramebuffer = gpu::CreateFramebuffer();

    renderbuffer->Bind();
    colorAttachment->Bind();
    colorAttachment->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    colorAttachment->SetMagFilter(gpu::MagTextureFilter::LINEAR);
    testFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);

    testFramebuffer->SetupColorAttachment(0, colorAttachment);
    testFramebuffer->SetupDepthStencilAttachment(renderbuffer);
    testFramebuffer->IsComplete();

    gpu::BindDefaultFramebuffer(gpu::FramebufferBindMode::READ_WRITE);

    // resources::MeshResource* backPackMesh = resources::AssimpLoadMesh("assets\\models\\backpack\\", "backpack.obj");

    resources::TextureResource* crateDiffuseMapResource = resources::LoadPNG("assets/textures/crate-diffuse-map.png");
    resources::TextureResource* crateSpecularMapResource = resources::LoadPNG("assets/textures/crate-specular-map.png");

    resources::TextureResource* brickWallDiffuseMapResource = resources::LoadJPEG("assets/textures/brick-diffuse-map.jpg");
    resources::TextureResource* brickWallNormalMapResource = resources::LoadJPEG("assets/textures/brick-normal-map.png");

    resources::TextureResource* blankTextureResource = resources::LoadPNG("assets/textures/blank.png");

    crateDiffuseMapResource->FreeMainMemory();
    crateSpecularMapResource->FreeMainMemory();

    brickWallDiffuseMapResource->FreeMainMemory();
    brickWallNormalMapResource->FreeMainMemory();

    blankTextureResource->FreeMainMemory();

    // backPackMesh->FreeMainMemory();

    auto crateDiffuseMap = crateDiffuseMapResource->TextureHandle;
    auto crateSpecularMap = crateSpecularMapResource->TextureHandle;

    auto brickWallDiffuseMap = brickWallDiffuseMapResource->TextureHandle;
    auto brickWallNormalMap = brickWallNormalMapResource->TextureHandle;

    auto blankTextureMap = blankTextureResource->TextureHandle;

    // auto backpackVao = backPackMesh->VAO;

    crateDiffuseMap->Bind();
    crateDiffuseMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    crateDiffuseMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    crateSpecularMap->Bind();
    crateSpecularMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    crateSpecularMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    brickWallDiffuseMap->Bind();
    brickWallDiffuseMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    brickWallDiffuseMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    brickWallNormalMap->Bind();
    brickWallNormalMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    brickWallNormalMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    blankTextureMap->Bind();
    blankTextureMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    blankTextureMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    window->Prepare();
    window->Show();

    gpu::Shader* blinnPhongShader = gpu::CompileShaderProgram({ platform::ReadFile("fwd_blinn_phong.vert", true) },
                                                              { platform::ReadFile("fwd_blinn_phong.frag", true) });

    gpu::Shader* blinnPhongMapsShader = gpu::CompileShaderProgram({ platform::ReadFile("fwd_blinn_phong_maps.vert", true) },
                                                                  { platform::ReadFile("fwd_blinn_phong_maps.frag", true) });

    gpu::Viewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    scene::Camera perspectiveCamera{ scene::CameraMode::PERSPECTIVE, { 0, 0, 2.5 } };
    perspectiveCamera.AspectRatio = window->GetAspectRatio();

    scene::ForwardBlinnPhongRenderer renderer{ 32, blinnPhongMapsShader };
    renderer.SetAmbientStrength(0.05);

    scene::RenderTarget renderTarget;
    renderTarget.Camera = &perspectiveCamera;
    renderTarget.Framebuffer = nullptr;
    renderTarget.Viewport = windowViewport;

    scene::BlinnPhongMaterial flatMaterial;
    flatMaterial.Shininess = 32;
    flatMaterial.DiffuseColor = glm::vec3(0.4, 0.2, 0.7);
    flatMaterial.SpecularColor = glm::vec3(0.4, 0.2, 0.7);
    flatMaterial.SetCustomShader(blinnPhongShader);

    scene::BlinnPhongMapsMaterial quadMaterial;
    quadMaterial.Shininess = 32;
    quadMaterial.DiffuseMap = brickWallDiffuseMap;
    quadMaterial.SpecularMap = blankTextureMap;
    quadMaterial.NormalMap = brickWallNormalMap;

    scene::BlinnPhongMapsMaterial cubeMaterial;
    cubeMaterial.Shininess = 32;
    cubeMaterial.DiffuseMap = crateDiffuseMap;
    cubeMaterial.SpecularMap = crateSpecularMap;
    cubeMaterial.NormalMap = blankTextureMap;

    scene::Renderable quad{ "Quad" };
    quad.Transform.Translation = { -2, 0, -4 };
    quad.Material = &quadMaterial;
    quad.VertexArray = misc::QuadVertexArray;
    quad.Type = scene::RenderableType::STATIC;
    glm::vec3 rotAxis = glm::normalize(glm::vec3(-1.0, 0.0, 0.0));
    quad.Transform.Rotation.x = rotAxis.x;
    quad.Transform.Rotation.y = rotAxis.y;
    quad.Transform.Rotation.z = rotAxis.z;

    scene::Renderable cube{ "Cube" };
    cube.Transform.Translation = { 2, 0, -2 };
    cube.Material = &flatMaterial;
    cube.VertexArray = misc::CubeVertexArray;
    cube.Type = scene::RenderableType::STATIC;
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

    scene::PointLight redLight;
    redLight.Position = { 0, 0, -2 };
    redLight.Color = { 1, 0, 0 };
    redLight.Constant = 1;
    redLight.Linear = 0.09;
    redLight.Quadratic = 0.032;

    scene::PointLight greenLight;
    greenLight.Position = { 0, 0, -2 };
    greenLight.Color = { 0, 1, 0 };
    greenLight.Constant = 1;
    greenLight.Linear = 0.09;
    greenLight.Quadratic = 0.032;

    scene::PointLight blueLight;
    blueLight.Position = { 0, 0, -2 };
    blueLight.Color = { 0, 0, 1 };
    blueLight.Constant = 1;
    blueLight.Linear = 0.09;
    blueLight.Quadratic = 0.032;

    scene::DirectionalLight whiteLight;
    whiteLight.Direction = { -0.5, 0, -0.3 };
    whiteLight.Color = { 1, 1, 1 };

    scene::RenderScene sceneToRender;
    sceneToRender.StaticGeometry.Add(&cube);
    sceneToRender.StaticGeometry.Add(&cube1);
    sceneToRender.StaticGeometry.Add(&cube2);
    sceneToRender.StaticGeometry.Add(&cube3);
    sceneToRender.StaticGeometry.Add(&cube4);
    sceneToRender.StaticGeometry.Add(&quad);
    // sceneToRender.Renderables.Add(backPackRenderable);
    sceneToRender.Lights.Add(&redLight);
    sceneToRender.Lights.Add(&greenLight);
    // sceneToRender.Lights.Add(&whiteLight);
    // sceneToRender.Lights.Add(&blueLight);
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
            quad.Transform.Rotation.w = glm::radians((float)(currentRotation % 360));
        }

        if (IsMouseButtonPressed(MouseButton::LEFT))
        {
            auto mousePos = GetMousePostion();

            if (mousePos.MouseMoved)
            {
                perspectiveCamera.AddRotation(-mousePos.DeltaX, mousePos.DeltaY);
            }
        }

        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
        renderer.Render(&sceneToRender, &renderTarget);
        window->Swap();
    }

    delete blinnPhongShader;

    window->Destroy();

    return 0;
}