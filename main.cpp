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

    lucid::StaticArray<lucid::math::vec3> rectangleVerts(4);
    rectangleVerts.Add({ 0.5f, 0.5f, 0 });
    rectangleVerts.Add({ 0.5f, -0.5f, 0 });
    rectangleVerts.Add({ -0.5f, -0.5f, 0 });
    rectangleVerts.Add({ -0.5f, 0.5f, 0 });

    lucid::StaticArray<uint32_t> rectangleIndices(6);
    rectangleIndices.Add(0);
    rectangleIndices.Add(1);
    rectangleIndices.Add(3);
    rectangleIndices.Add(1);
    rectangleIndices.Add(2);
    rectangleIndices.Add(3);

    lucid::gpu::BufferDescription bufferDescription;
    bufferDescription.data = rectangleVerts;
    bufferDescription.size = rectangleVerts.SizeInBytes;
    bufferDescription.offset = 0;

    lucid::gpu::Buffer* vertexBuffer =
    lucid::gpu::CreateBuffer(bufferDescription, lucid::gpu::BufferUsage::STATIC);

    bufferDescription.data = rectangleIndices;
    bufferDescription.size = rectangleIndices.SizeInBytes;

    lucid::gpu::Buffer* elementBuffer =
    lucid::gpu::CreateBuffer(bufferDescription, lucid::gpu::BufferUsage::STATIC);

    lucid::StaticArray<lucid::gpu::VertexAttribute> attributes(1);
    attributes.Add({ 0, 3, lucid::Type::FLOAT, false, 3 * sizeof(float), 0 });

    lucid::gpu::VertexArray* vertexArray =
    lucid::gpu::CreateVertexArray(&attributes, vertexBuffer, elementBuffer);

    attributes.Free();

    lucid::gpu::Shader* simpleShader =
    lucid::gpu::CompileShaderProgram({ VERTEX_SOURCE }, { FRAGMENT_SOURCE });

    simpleShader->Use();
    simpleShader->SetVector("triangleColor", lucid::math::vec3{ 0.6, 0.7, 0.3 });
    vertexArray->Bind();

    window->ClearColor();
    lucid::gpu::DrawElement(lucid::Type::UINT_32, 6, lucid::gpu::DrawMode::TRIANGLES);
    window->Swap();

    getchar();

    delete vertexArray;
    delete simpleShader;

    vertexBuffer->Free();
    elementBuffer->Free();

    window->Destroy();

    return 0;
}