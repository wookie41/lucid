#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"

namespace lucid::gpu
{
    class CCubemap;
    struct FGPUState;
};

namespace lucid::scene
{
    class CLight;
    struct FRenderable;

    struct FSkybox
    {
        gpu::CCubemap* SkyboxCubemap = nullptr;
    };

    FSkybox CreateSkybox(const FArray<FString>& InSkyboxFacesPaths, const FANSIString& InName);

    /*
    * The RenderScene contains things like objects to render, lights, fog voluems
    * in a Renderer-implementation-agnostic format. The specific Renderers then
    * use the provided scene information to render the scene in their own specific way
    */
    struct FRenderScene
    {
        FLinkedList<FRenderable> StaticGeometry;
        FLinkedList<FRenderable> DynamicGeometry;
        FLinkedList<CLight> Lights;
        FSkybox* SceneSkybox = nullptr;
    };
} // namespace lucid::scene
