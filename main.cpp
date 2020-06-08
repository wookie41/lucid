#include "devices/gpu/gpu.hpp"
#include "platform/window.hpp"
#include "devices/gpu/buffer.hpp"

#include "stdio.h"
#include "GL/glew.h"
#include "stdlib.h"
#include "common/collections.hpp"

int main(int argc, char** argv)
{
    lucid::gpu::Init();
    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });

    window->Prepare();
    window->Show();

    window->SetClearColor({ .2f, .3f, .8f, 1.f });
    window->ClearColor();
    window->Swap();

    lucid::StaticArray<float> testData(5);
    
    testData.Add(1);
    testData.Add(2);
    testData.Add(3);
    testData.Add(4);
    testData.Add(5);

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
    
    lucid::StaticArray<float> testDataDownloaded(5);    

    buffer->Download(testDataDownloaded);
    testDataDownloaded.Length = 5;
    
    for (uint32_t i = 0; i < testDataDownloaded.Length; ++i)
        printf("%f\n", *testDataDownloaded[i]);

    buffer->Free();
    window->Destroy();

    delete window;
    return 0;
}