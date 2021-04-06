#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"

namespace lucid::scene
{
    class CLight;
    class CStaticMesh;
    class CSkybox;
    
    CSkybox* CreateSkybox(const u32& RenderableId, const FArray<FString>& InSkyboxFacesPaths, const FANSIString& InName);

    /*
     * The RenderScene contains things like objects to render, lights, fog volumes in a Renderer-implementation-agnostic format.
     * It's a result of running culling techniques which produce a minimal set of objects that the renderer needs to render the
     * scene The specific Renderers then use the provided scene information to render the scene in their own specific way
     */
    struct FRenderScene
    {
        FRenderScene() = default;

        CStaticMesh**   StaticMeshes    = nullptr;
        CLight**        Lights          = nullptr;
        CSkybox*        Skybox          = nullptr;
    };
} // namespace lucid::scene
