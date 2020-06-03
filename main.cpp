#include "devices/gpu/include/gpu.hpp"
#include "platform/include/window.hpp"

#include "stdio.h"
#include "GL/glew.h"

int main(int argc, char** argv)
{
    lucid::gpu::Init();
    lucid::platform::Window* window = lucid::platform::CreateWindow({ "Lucid", 200, 200, 800, 600 });

    window->Prepare();
    window->Show();

    window->SetClearColor({ .2f, .3f, .8f, 1.f });
    window->ClearColor();
    window->Swap();

    getchar();

    window->Destroy();
    delete window;

    return 0;
}