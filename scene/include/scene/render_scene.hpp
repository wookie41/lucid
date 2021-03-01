#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"

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
    class Renderable;

    // Skybox

    struct Skybox
    {
        gpu::Cubemap* SkyboxCubemap = nullptr;
    };

    Skybox CreateSkybox(const StaticArray<String>& InSkyboxFacesPaths);

    ////////////////////////////////////////////////////////////

    // Render scene

    struct RenderScene
    {
        LinkedList<Renderable> StaticGeometry;
        LinkedList<Renderable> DynamicGeometry;
        LinkedList<Light> Lights;
        Skybox* SceneSkybox = nullptr;
    };

    ////////////////////////////////////////////////////////////

} // namespace lucid::scene
