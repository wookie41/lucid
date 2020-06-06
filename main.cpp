#include "devices/gpu/include/gpu.hpp"
#include "platform/include/window.hpp"
#include "devices/gpu/include/buffer.hpp"

#include "stdio.h"
#include "GL/glew.h"
#include "stdlib.h"

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

    lucid::gpu::Buffer* buffer =
    lucid::gpu::CreateImmutableBuffer(bufferDescription,
                                      ((uint16_t)lucid::gpu::ImmutableBufferUsage::READ) |
                                      ((uint16_t)lucid::gpu::ImmutableBufferUsage::WRITE) |
                                      ((uint16_t)lucid::gpu::ImmutableBufferUsage::PERSISTENT) |
                                      ((uint16_t)lucid::gpu::ImmutableBufferUsage::COHERENT));


    float* bufferPtr = (float*)buffer->MemoryMap((uint16_t)lucid::gpu::BufferAccessPolicy::READ |
                                                 (uint16_t)lucid::gpu::BufferAccessPolicy::PERSISTENT);

    for (uint32_t i = 0; i < sizeof(testData) / sizeof(float); ++i)
    {
        bufferPtr[i] += 1;
    }

    buffer->Download(testDataDownloaded);

    for (uint32_t i = 0; i < sizeof(testData) / sizeof(float); ++i)
    {
        printf("%f ", testDataDownloaded[i]);
    }

    puts("");

    buffer->Free();
    window->Destroy();

    delete window;
    return 0;
}