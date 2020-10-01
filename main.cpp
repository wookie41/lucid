#include "platform/window.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/init.hpp"
#include "graphics/static_mesh.hpp"
#include "devices/gpu/texture.hpp"
#include "platform/fs.hpp"
#include "canvas/canvas.hpp"
#include "platform/input.hpp"
#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/gpu.hpp"
#include "stdio.h"
#include "GL/glew.h"
#include "stdlib.h"
#include "devices/gpu/vao.hpp"

int main(int argc, char** argv)
{
    if (lucid::gpu::Init({}) < 0)
    {
        puts("Failed to init GPU");
        return -1;
    }

    lucid::gpu::InitTextures();

    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });
    lucid::graphics::InitBasicShapes();

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

    lucid::math::ivec3 containerTextureSize = containerTexture->GetDimensions();
    lucid::math::ivec3 faceTextureSize = faceTexture->GetDimensions();

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

    lucid::math::mat4 ProjectionMatrix =
    lucid::math::CreateOrthographicProjectionMatrix(800, 0, 0, 600, 0.1f, 1000.0f);

    lucid::gpu::Shader* simpleShader =
    lucid::gpu::CompileShaderProgram({ lucid::platform::ReadFile("sprite.vert", true) },
                                     { lucid::platform::ReadFile("sprite.frag", true) });

    simpleShader->Use();
    simpleShader->SetMatrix("Projection", ProjectionMatrix);

    lucid::gpu::Viewport windowViewport{ 0, 0, 800, 600 };
    lucid::gpu::Viewport framebufferViewort{ 0, 0, 400, 300 };

    lucid::gpu::DisableDepthTest();
    bool isRunning = true;
    lucid::graphics::QuadVertexArray->Bind();
    while (isRunning)
    {
        lucid::ReadEvents();

        if (lucid::WasKeyPressed(SDLK_ESCAPE))
        {
            isRunning = false;
            break;
        }

        if (lucid::WasKeyPressed(SDLK_d))
        {
            faceTexture->Bind();
            faceTexture->SetWrapSFilter(lucid::gpu::WrapTextureFilter::CLAMP_TO_EDGE);
            faceTexture->Unbind();
        }

        if (lucid::WasKeyPressed(SDLK_a))
        {
            faceTexture->Bind();
            faceTexture->SetWrapSFilter(lucid::gpu::WrapTextureFilter::REPEAT);
            faceTexture->Unbind();
        }

        if (lucid::IsMouseButtonPressed(lucid::MouseButton::LEFT))
        {
            sprite2.Position = { (float)lucid::GetMousePostion().x,
                                 (float)window->GetSize().y - lucid::GetMousePostion().y };
        }

        if (lucid::WasMouseButtonPressed(lucid::MouseButton::RIGHT))
        {
            sprite2.Position = { 0, 0 };
        }

        sprite2.TextureToUse = faceTexture;
        sprite2.TextureRegionSize = { faceTexture->GetDimensions().x + 300,
                                      faceTexture->GetDimensions().y };

        // Draw to the framebuffer

        testFramebuffer->Bind(lucid::gpu::FramebufferBindMode::READ_WRITE);
        lucid::gpu::SetViewprot(framebufferViewort);
        lucid::gpu::SetClearColor(lucid::RedColor);
        lucid::gpu::ClearBuffers((lucid::gpu::ClearableBuffers)(lucid::gpu::ClearableBuffers::COLOR));

        canvasRoot.Draw(simpleShader);

        testFramebuffer->Unbind();

        // Draw the main framebuffer's contents to the screen

        lucid::gpu::SetViewprot(windowViewport);
        lucid::gpu::BindDefaultFramebuffer(lucid::gpu::FramebufferBindMode::READ_WRITE);
        lucid::gpu::SetClearColor(lucid::BlackColor);
        lucid::gpu::ClearBuffers((lucid::gpu::ClearableBuffers)(lucid::gpu::ClearableBuffers::COLOR));

        sprite2.TextureToUse = colorAttachment;
        sprite2.TextureRegionSize = { colorAttachment->GetDimensions().x,
                                      colorAttachment->GetDimensions().y };

        canvasRoot.Draw(simpleShader);
        window->Swap();
    }

    delete simpleShader;

    window->Destroy();

    return 0;
}