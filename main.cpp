#include "devices/gpu/include/gpu.hpp"
#include "platform/include/window.hpp"
#include "devices/gpu/include/buffer.hpp"

#include "stdio.h"
#include "GL/glew.h"

float testData[] = { 1.f, 2.f, 3.f, 4.f, 5.f };
float testDataDownloaded[] = { 0.f, 0.f, 0.f, 0.f, 0.f };

int main(int argc, char** argv)
{
    lucid::gpu::Init();
    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });

    window->Prepare();
    window->Show();

    window->SetClearColor({ .2f, .3f, .8f, 1.f });
    window->ClearColor();
    window->Swap();

    lucid::gpu::BufferDescription bufferDescription;
    bufferDescription.data = testData;
    bufferDescription.size = sizeof(testData);
    bufferDescription.offset = 0;

    lucid::gpu::Buffer* buffer = lucid::gpu::CreateBuffer(bufferDescription, lucid::gpu::BufferUsage::STATIC);
    

    buffer->Download(testDataDownloaded);

    //TODO test buffer mapping

    buffer->Free();

    // gestchar();

    window->Destroy();
    delete window;

    return 0;
}