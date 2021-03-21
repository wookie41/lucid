#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"

namespace lucid::gpu
{
    class CCubemap;
};

namespace lucid::scene
{
    // The RenderScene contains thing like objects to render, lights, fog voluems
    // in a Renderer-implementation-agnostic format. The specific Renderers then
    // use the provided scene information to render the scene in their own specific way

    class CLight;
    struct FRenderable;

    // Skybox

    struct FSkybox
    {
        gpu::CCubemap* SkyboxCubemap = nullptr;
    };

    FSkybox CreateSkybox(const FArray<FString>& InSkyboxFacesPaths);

    ////////////////////////////////////////////////////////////

    // Render scene

    struct FRenderScene
    {
        FLinkedList<FRenderable> StaticGeometry;
        FLinkedList<FRenderable> DynamicGeometry;
        FLinkedList<CLight> Lights;
        FSkybox* SceneSkybox = nullptr;
    };

    ////////////////////////////////////////////////////////////

} // namespace lucid::scene
