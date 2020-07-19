#include "platform/window.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/gpu.hpp"
#include "graphics/static_mesh.hpp"
#include "devices/gpu/textures.hpp"
#include "platform/fs.hpp"
#include "canvas/canvas.hpp"
#include "platform/input.hpp"

#include "stdio.h"
#include "GL/glew.h"
#include "stdlib.h"


int main(int argc, char** argv)
{
    lucid::gpu::Init();
    lucid::gpu::InitTextures();

    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });
    lucid::graphics::InitBasicShapes();

    lucid::gpu::Texture* containerTexture = lucid::gpu::CreateTextureFromJPEG("container.jpg");
    lucid::gpu::Texture* faceTexture = lucid::gpu::CreateTextureFromPNG("awesomeface.png");

    lucid::math::ivec3 containerTextureSize = containerTexture->GetSize();
    lucid::math::ivec3 faceTextureSize = faceTexture->GetSize();

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
    window->SetClearColor({ .2f, .3f, .8f, 1.f });

    lucid::math::mat4 ProjectionMatrix =
    lucid::math::CreateOrthographicProjectionMatrix(800, 0, 0, 600, 0.1f, 1000.0f);

    lucid::gpu::Shader* simpleShader =
    lucid::gpu::CompileShaderProgram({ lucid::platform::ReadFile("sprite.vert", true) },
                                     { lucid::platform::ReadFile("sprite.frag", true) });

    simpleShader->Use();
    simpleShader->SetMatrix("Projection", ProjectionMatrix);

    bool isRunning = true;
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


        window->ClearColor();
        canvasRoot.Draw(simpleShader);
        window->Swap();
    }

    delete simpleShader;

    window->Destroy();

    return 0;
}