#include "devices/gpu/gpu.hpp"
#include "platform/window.hpp"
#include "devices/gpu/buffer.hpp"

#include "stdio.h"
#include "GL/glew.h"
#include "stdlib.h"
#include "common/collections.hpp"
#include "devices/gpu/shader.hpp"

static char* VERTEX_SOURCE = "#version 330 core\n"
                             "layout(location = 0) in vec3 position;\n"
                             "void main()\n"
                             "{\n"
                             "    gl_Position = vec4(position, 1);\n"
                             "}\n";

static char* FRAGMENT_SOURCE = "#version 330 core\n"
                               "out vec4 FragColor;\n"
                               "uniform vec3 triangleColor;\n"
                               "void main() { FragColor = vec4(triangleColor, 1.0f); }\n";

int main(int argc, char** argv)
{
    lucid::gpu::Init();
    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });

    window->Prepare();
    window->Show();
    window->SetClearColor({ .2f, .3f, .8f, 1.f });

    lucid::StaticArray<lucid::math::vec3> triangleVerts(3);

    triangleVerts.Add({ -.5f, -.5f, 0 });
    triangleVerts.Add({ .5f, -.5f, 0 });
    triangleVerts.Add({ 0, .5f, 0 });

    lucid::gpu::BufferDescription bufferDescription;
    bufferDescription.data = triangleVerts;
    bufferDescription.size = triangleVerts.SizeInBytes;
    bufferDescription.offset = 0;

    lucid::gpu::Buffer* buffer = lucid::gpu::CreateBuffer(bufferDescription, lucid::gpu::BufferUsage::STATIC);

    lucid::gpu::Shader* simpleShader =
    lucid::gpu::CompileShaderProgram({ VERTEX_SOURCE }, { FRAGMENT_SOURCE });

    simpleShader->Use();
    simpleShader->SetVector("triangleColor", lucid::math::vec3 { 0.6, 0.7, 0.3 } );

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    buffer->Bind(lucid::gpu::BufferBindPoint::VERTEX);

    lucid::gpu::AddVertexAttribute({ 0, 3, lucid::Type::FLOAT, false, 3 * sizeof(float), 0 });

    while (true)
    {
        window->ClearColor();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        window->Swap();
    }

    getchar();

    buffer->Free();
    window->Destroy();

    delete window;
    return 0;
}