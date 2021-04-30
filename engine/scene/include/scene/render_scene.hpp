#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"

namespace lucid::resources
{
    class CTextureResource;
}

namespace lucid::scene
{
    class CLight;
    class CDirectionalLight;
    class CSpotLight;
    class CPointLight;
    class CStaticMesh;
    class CSkybox;


    /*
     * The RenderScene contains things like objects to render, lights, fog volumes in a Renderer-implementation-agnostic format.
     * It's a result of running culling techniques which produce a minimal set of objects that the renderer needs to render the
     * scene The specific Renderers then use the provided scene information to render the scene in their own specific way
     */
    struct FRenderScene
    {
        FRenderScene() = default;

        CStaticMesh**       StaticMeshes = nullptr;
        CDirectionalLight** DirectionalLights = nullptr;
        CSpotLight**        SpotLights = nullptr;
        CPointLight**       PointLights = nullptr;
        CLight**            AllLights = nullptr;
        CSkybox*            Skybox = nullptr;
    };
} // namespace lucid::scene
