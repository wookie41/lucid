#pragma once

#include "common/collections.hpp"

namespace lucid::gpu
{
    class Cubemap;
};

namespace lucid::scene
{
    // The RenderScene contains thing like objects to render, lights, fog voluems
    // in a Renderer-implementation-agnostic format. The specific Renderers then
    // use the provided scene information to render the scene in their own specific way

    class Light;

    // Skybox

    struct Skybox
    {
        gpu::Cubemap* SkyboxCubemap = nullptr;
    };

    Skybox CreateSkybox(const char* FacesPaths[6]);

    ////////////////////////////////////////////////////////////

    // Render scene

    struct RenderScene
    {
        LinkedList<class Renderable> StaticGeometry;
        LinkedList<class Renderable> DynamicGeometry;
        LinkedList<Light> Lights;
        Skybox SceneSkybox;
    };

    ////////////////////////////////////////////////////////////

} // namespace lucid::scene
