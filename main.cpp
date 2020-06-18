#include "platform/window.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/gpu.hpp"
#include "graphics/static_mesh.hpp"
#include "devices/gpu/textures.hpp"
#include "platform/fs.hpp"

#include "canvas/canvas.hpp"

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
    lucid::math::ivec3 textureSize = containerTexture->GetSize();

    lucid::canvas::Sprite sprite;
    sprite.Position = { 0, 0 };
    sprite.SpriteSize = { 200, 200 };
    sprite.TextureRegionSize = { textureSize.x, textureSize.y };
    sprite.TextureToUse = containerTexture;

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

    while (true)
    {
        window->ClearColor();
        sprite.Draw(simpleShader);
        window->Swap();
    }

    getchar();

    delete simpleShader;

    window->Destroy();

    return 0;
}