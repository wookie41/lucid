#include "platform/window.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/gpu.hpp"
#include "graphics/static_mesh.hpp"
#include "devices/gpu/textures.hpp"

#include "stdio.h"
#include "GL/glew.h"
#include "stdlib.h"

static char* VERTEX_SOURCE = "#version 330 core\n"
                             "layout(location = 0) in vec3 position;\n"
                             "layout(location = 1) in vec2 texCoords;\n"
                             "out vec2 TexCoords;\n"
                             "void main()\n"
                             "{\n"
                             "    TexCoords = texCoords;\n"
                             "    gl_Position = vec4(position * 0.75, 1);\n"
                             "}\n";

static char* FRAGMENT_SOURCE =
"#version 330 core\n"
"in vec2 TexCoords;\n"
"out vec4 FragColor;\n"
"uniform sampler2D textureToUse;\n"
"void main() { FragColor = texture(textureToUse, TexCoords); }\n";

int main(int argc, char** argv)
{
    lucid::gpu::Init();

    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });
    lucid::graphics::InitBasicShapes();
    lucid::gpu::Texture* containerTexture = lucid::gpu::CreateTextureFromJPEG("container.jpg");

    window->Prepare();
    window->Show();
    window->SetClearColor({ .2f, .3f, .8f, 1.f });

    lucid::gpu::Shader* simpleShader =
    lucid::gpu::CompileShaderProgram({ VERTEX_SOURCE }, { FRAGMENT_SOURCE });

    simpleShader->Use();
    // simpleShader->SetVector("triangleColor", lucid::math::vec3{ 0.6, 0.7, 0.3 });
    simpleShader->SetInt("textureToUse", 0);

    containerTexture->Bind();
    
    window->ClearColor();
    lucid::graphics::DrawMesh(&lucid::graphics::QuadShape);
    window->Swap();

    getchar();

    delete simpleShader;

    window->Destroy();

    return 0;
}