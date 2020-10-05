#include "platform/window.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/init.hpp"
#include "misc/basic_shapes.hpp"
#include "devices/gpu/texture.hpp"
#include "platform/fs.hpp"
#include "canvas/canvas.hpp"
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

int main(int argc, char** argv)
{
    if (lucid::gpu::Init({}) < 0)
    {
        puts("Failed to init GPU");
        return -1;
    }

    lucid::gpu::InitTextures();

    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 900, 420, 800, 600 });
    lucid::misc::InitBasicShapes();

    lucid::gpu::Texture* colorAttachment = lucid::gpu::Create2DTexture(nullptr, { 800, 600 }, 0, false);
    lucid::gpu::FramebufferAttachment* renderbuffer =
    lucid::gpu::CreateRenderbuffer(lucid::gpu::RenderbufferFormat::DEPTH24_STENCIL8, 800, 600);
    lucid::gpu::Framebuffer* testFramebuffer = lucid::gpu::CreateFramebuffer();

    renderbuffer->Bind();
    colorAttachment->Bind();
    colorAttachment->SetMinFilter(lucid::gpu::MinTextureFilter::LINEAR);
    colorAttachment->SetMagFilter(lucid::gpu::MagTextureFilter::LINEAR);
    testFramebuffer->Bind(lucid::gpu::FramebufferBindMode::READ_WRITE);

    testFramebuffer->SetupColorAttachment(0, colorAttachment);
    testFramebuffer->SetupDepthStencilAttachment(renderbuffer);
    testFramebuffer->IsComplete();

    colorAttachment->Unbind();
    renderbuffer->Unbind();
    testFramebuffer->Unbind();

    lucid::gpu::BindDefaultFramebuffer(lucid::gpu::FramebufferBindMode::READ_WRITE);

    lucid::gpu::Texture* containerTexture = lucid::gpu::CreateTextureFromJPEG("container.jpg");
    lucid::gpu::Texture* faceTexture = lucid::gpu::CreateTextureFromPNG("awesomeface.png");

    glm::ivec3 containerTextureSize = containerTexture->GetDimensions();
    glm::ivec3 faceTextureSize = faceTexture->GetDimensions();

    lucid::canvas::CanvasItem canvasRoot;
    canvasRoot.Position = { 400, 300 };

    lucid::canvas::Sprite sprite1;
    sprite1.Position = { 200, 0 };
    sprite1.SpriteSize = { 200, 200 };
    sprite1.TextureRegionSize = { containerTextureSize.x, containerTextureSize.y };
    sprite1.TextureToUse = containerTexture;

    lucid::canvas::Sprite sprite2;
    sprite2.Position = { 0, 100 };
    sprite2.SpriteSize = { 200, 200 };
    sprite2.TextureRegionSize = { faceTextureSize.x + 300, faceTextureSize.y };
    sprite2.TextureToUse = faceTexture;
    sprite2.RespectParentPosition = false;

    canvasRoot.IsVisible = true;
    canvasRoot.AddChild(&sprite1);
    canvasRoot.AddChild(&sprite2);

    window->Prepare();
    window->Show();

    lucid::gpu::Shader* spriteShader =
    lucid::gpu::CompileShaderProgram({ lucid::platform::ReadFile("screen.vert", true) },
                                     { lucid::platform::ReadFile("texture.frag", true) });

    lucid::gpu::Shader* blinnPhongShader =
    lucid::gpu::CompileShaderProgram({ lucid::platform::ReadFile("fwd_blinn_phong.vert", true) },
                                     { lucid::platform::ReadFile("fwd_blinn_phong.frag", true) });

    lucid::gpu::Viewport windowViewport{ 0, 0, window->GetWidth(), window->GetHeight() };
    lucid::gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    lucid::scene::Camera perspectiveCamera{ lucid::scene::CameraMode::PERSPECTIVE, { 0, 0, 2.5 } };
    perspectiveCamera.AspectRatio = window->GetAspectRatio();

    lucid::scene::Camera orthographicCamera{ lucid::scene::CameraMode::ORTHOGRAPHIC };
    orthographicCamera.Left = 0;
    orthographicCamera.Right = window->GetWidth();
    orthographicCamera.Bottom = 0;
    orthographicCamera.Top = window->GetHeight();

    lucid::scene::ForwardBlinnPhongRenderer renderer{ 32, blinnPhongShader };

    lucid::scene::RenderTarget renderTarget;
    renderTarget.Camera = &perspectiveCamera;
    renderTarget.Framebuffer = nullptr;
    renderTarget.Viewport = windowViewport;

    spriteShader->Use();
    spriteShader->SetMatrix("Projection", orthographicCamera.GetProjectionMatrix());
    spriteShader->SetMatrix("View", orthographicCamera.GetViewMatrix());

    lucid::scene::BlinnPhongMaterial cubeMaterial1;
    cubeMaterial1.Shininess = 64.f;
    cubeMaterial1.DiffuseColor = { 0.2, 0.3, 0.4 };
    cubeMaterial1.SpecularColor = { 1, 1, 1 };

    lucid::scene::BlinnPhongMaterial cubeMaterial2;
    cubeMaterial2.Shininess = 128.f;
    cubeMaterial2.DiffuseColor = { 0.6, 0.3, 0.1 };
    cubeMaterial2.SpecularColor = { 1, 1, 1 };

    lucid::scene::Renderable cube1{ "Cube1" };
    cube1.Transform.Translation = { -2, 0, -2 };
    cube1.Material = &cubeMaterial1;
    cube1.VertexArray = lucid::misc::CubeVertexArray;
    cube1.Type = lucid::scene::RenderableType::STATIC;

    lucid::scene::Renderable cube2{ "Cube2" };
    cube2.Transform.Translation = { 2, 0, -2 };
    cube2.Material = &cubeMaterial2;
    cube2.VertexArray = lucid::misc::CubeVertexArray;
    cube2.Type = lucid::scene::RenderableType::STATIC;

    lucid::scene::DirectionalLight light1;
    light1.Direction = { 0, 0, -1 };
    light1.Color = { 1, 0, 0 };

    lucid::scene::DirectionalLight light2;
    light2.Direction = { 1, 0, -1 };
    light2.Color = { 0, 1, 0 };

    lucid::scene::DirectionalLight light3;
    light3.Direction = { -1, 0, -1 };
    light3.Color = { 0, 0, 1 };

    lucid::scene::RenderScene sceneToRender;
    sceneToRender.Renderables.Add(&cube1);
    sceneToRender.Renderables.Add(&cube2);
    sceneToRender.DirectionalLights.Add(&light1);
    sceneToRender.DirectionalLights.Add(&light2);
    sceneToRender.DirectionalLights.Add(&light3);

    bool isRunning = true;
    while (isRunning)
    {
        lucid::ReadEvents();

        if (lucid::WasKeyPressed(SDLK_ESCAPE))
        {
            isRunning = false;
            break;
        }

        if (lucid::IsKeyPressed(SDLK_w))
        {
            perspectiveCamera.MoveForward(1.f / 60.f);
        }

        if (lucid::IsKeyPressed(SDLK_s))
        {
            perspectiveCamera.MoveBackward(1.f / 60.f);
        }

        if (lucid::IsKeyPressed(SDLK_a))
        {
            perspectiveCamera.MoveLeft(1.f / 60.f);
        }

        if (lucid::IsKeyPressed(SDLK_d))
        {
            perspectiveCamera.MoveRight(1.f / 60.f);
        }

        if (lucid::IsMouseButtonPressed(lucid::MouseButton::LEFT))
        {
            auto mousePos = lucid::GetMousePostion();

            if (mousePos.MouseMoved)
            {
                perspectiveCamera.AddRotation(-mousePos.DeltaX, mousePos.DeltaY);
            }
        }

        if (lucid::IsMouseButtonPressed(lucid::MouseButton::RIGHT))
        {
            sprite2.Position = { (float)lucid::GetMousePostion().X,
                                 (float)window->GetHeight() - lucid::GetMousePostion().Y };
        }

        if (lucid::WasMouseButtonPressed(lucid::MouseButton::RIGHT))
        {
            sprite2.Position = { 0, 0 };
        }

        // Draw the cube in 3D using perspective projection //

        lucid::gpu::EnableDepthTest();
        lucid::gpu::SetClearColor(lucid::BlackColor);
        lucid::gpu::ClearBuffers((lucid::gpu::ClearableBuffers)(lucid::gpu::ClearableBuffers::COLOR |
                                                                lucid::gpu::ClearableBuffers::DEPTH));

        renderer.Render(&sceneToRender, &renderTarget);

        // Draw the 2D sprites on top of it using orthograpic camera //

        spriteShader->Use();
        lucid::gpu::DisableDepthTest();

        // Draw to the framebuffer

        sprite2.TextureToUse = faceTexture;
        sprite2.TextureRegionSize = { faceTexture->GetDimensions().x, faceTexture->GetDimensions().y };

        testFramebuffer->Bind(lucid::gpu::FramebufferBindMode::READ_WRITE);
        lucid::gpu::SetViewport(framebufferViewort);
        lucid::gpu::SetClearColor(lucid::RedColor);
        lucid::gpu::ClearBuffers((lucid::gpu::ClearableBuffers)(lucid::gpu::ClearableBuffers::COLOR));

        canvasRoot.Draw(spriteShader);

        testFramebuffer->Unbind();

        // Draw to the default framebuffer

        lucid::gpu::SetViewport(windowViewport);
        lucid::gpu::BindDefaultFramebuffer(lucid::gpu::FramebufferBindMode::READ_WRITE);

        sprite2.TextureToUse = colorAttachment;
        sprite2.TextureRegionSize = { colorAttachment->GetDimensions().x,
                                      colorAttachment->GetDimensions().y };

        canvasRoot.Draw(spriteShader);

        window->Swap();
    }

    delete spriteShader;
    delete blinnPhongShader;

    window->Destroy();

    return 0;
}