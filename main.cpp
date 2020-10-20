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

    gpu::Texture* colorAttachment = gpu::CreateEmpty2DTexture(window->GetWidth(), window->GetHeight(), gpu::TextureFormat::RGBA, 0);
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

    resources::TextureResource* diffuseTextureResource = resources::LoadPNG("assets/textures/cube-diffuse-map.png");
    resources::TextureResource* specularTextureResource = resources::LoadPNG("assets/textures/cube-specular-map.png");
    resources::MeshResource* backPackMesh = resources::AssimpLoadMesh("assets\\models\\backpack\\", "backpack.obj");

    diffuseTextureResource->FreeMainMemory();
    specularTextureResource->FreeMainMemory();
    backPackMesh->FreeMainMemory();

    auto cubeDiffuseMap = diffuseTextureResource->TextureHandle;
    auto cubeSpecularMap = specularTextureResource->TextureHandle;
    auto backpackVao = backPackMesh->VAO;

    cubeDiffuseMap->Bind();
    cubeDiffuseMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    cubeDiffuseMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    cubeSpecularMap->Bind();
    cubeSpecularMap->SetMinFilter(gpu::MinTextureFilter::LINEAR);
    cubeSpecularMap->SetMagFilter(gpu::MagTextureFilter::LINEAR);

    window->Prepare();
    window->Show();

    gpu::Shader* blinnPhongShader = gpu::CompileShaderProgram({ platform::ReadFile("fwd_blinn_phong.vert", true) },
                                                              { platform::ReadFile("fwd_blinn_phong.frag", true) });

    gpu::Shader* blinnPhongMapsShader = gpu::CompileShaderProgram({ platform::ReadFile("fwd_blinn_phong.vert", true) },
                                                                  { platform::ReadFile("fwd_blinn_phong_maps.frag", true) });

    gpu::Viewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    scene::Camera perspectiveCamera{ scene::CameraMode::PERSPECTIVE, { 0, 0, 2.5 } };
    perspectiveCamera.AspectRatio = window->GetAspectRatio();

    scene::ForwardBlinnPhongRenderer renderer{ 32, blinnPhongMapsShader };

    scene::RenderTarget renderTarget;
    renderTarget.Camera = &perspectiveCamera;
    renderTarget.Framebuffer = nullptr;
    renderTarget.Viewport = windowViewport;

    scene::BlinnPhongMaterial cubeMaterial1;
    cubeMaterial1.Shininess = 64.f;
    cubeMaterial1.DiffuseColor = { 0.2, 0.3, 0.4 };
    cubeMaterial1.SpecularColor = { 1, 1, 1 };
    cubeMaterial1.SetCustomShader(blinnPhongShader);

    scene::BlinnPhongMapsMaterial cubeMaterial2;
    cubeMaterial2.Shininess = 128.f;
    cubeMaterial2.DiffuseMap = cubeDiffuseMap;
    cubeMaterial2.SpecularMap = cubeSpecularMap;

    scene::Renderable cube1{ "Cube1" };
    cube1.Transform.Translation = { -2, 0, -2 };
    cube1.Material = &cubeMaterial1;
    cube1.VertexArray = misc::CubeVertexArray;
    cube1.Type = scene::RenderableType::STATIC;

    scene::Renderable cube2{ "Cube2" };
    cube2.Transform.Translation = { 2, 0, -2 };
    cube2.Material = &cubeMaterial2;
    cube2.VertexArray = misc::CubeVertexArray;
    cube2.Type = scene::RenderableType::STATIC;

    scene::Renderable* backPackRenderable = scene::CreateBlinnPhongRenderable("MyMesh", backPackMesh);

    scene::DirectionalLight light1;
    light1.Direction = { 0, -2, -2 };
    light1.Color = { 1, 0, 0 };

    scene::DirectionalLight light2;
    light2.Direction = { 1, 0, -1 };
    light2.Color = { 0, 1, 0 };

    scene::DirectionalLight light3;
    light3.Direction = { -1, 0, -1 };
    light3.Color = { 0, 0, 1 };

    scene::RenderScene sceneToRender;
    // sceneToRender.Renderables.Add(&cube1);
    // sceneToRender.Renderables.Add(&cube2);
    sceneToRender.Renderables.Add(backPackRenderable);
    sceneToRender.DirectionalLights.Add(&light1);
    sceneToRender.DirectionalLights.Add(&light2);
    sceneToRender.DirectionalLights.Add(&light3);

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

        if (IsMouseButtonPressed(MouseButton::LEFT))
        {
            auto mousePos = GetMousePostion();

            if (mousePos.MouseMoved)
            {
                perspectiveCamera.AddRotation(-mousePos.DeltaX, mousePos.DeltaY);
            }
        }

        gpu::EnableDepthTest();
        gpu::SetClearColor(BlackColor);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));

        renderer.Render(&sceneToRender, &renderTarget);

        window->Swap();
    }

    delete blinnPhongShader;

    window->Destroy();

    return 0;
}