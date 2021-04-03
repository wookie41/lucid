#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "devices/gpu/texture.hpp"
#include "renderable/mesh_renderable.hpp"

namespace lucid::gpu
{
    class CCubemap;
    struct FGPUState;
}; // namespace lucid::gpu

namespace lucid::scene
{
    class CLight;

    class CSkybox : public IRenderable

    {
      public:
        explicit CSkybox(const FDString& InName, const IRenderable* InParent, gpu::CCubemap* InSkyboxCubemap)
        : IRenderable(InName, InParent), SkyboxCubemap(InSkyboxCubemap)
        {
        }

        gpu::CCubemap* GetCubemap() const { return SkyboxCubemap; }

      private:
        gpu::CCubemap* SkyboxCubemap = nullptr;
    };

    CSkybox CreateSkybox(const u32& RenderableId, const FArray<FString>& InSkyboxFacesPaths, const FANSIString& InName);

    /*
     * The RenderScene contains things like objects to render, lights, fog volumes in a Renderer-implementation-agnostic format.
     * It's a result of running culling techniques which produce a minimal set of objects that the renderer needs to render the
     * scene The specific Renderers then use the provided scene information to render the scene in their own specific way
     */
    struct FRenderScene
    {
      public:
        FRenderScene() = default;

        CStaticMesh**   StaticMeshes    = nullptr;
        CLight**        Lights          = nullptr;
        CSkybox*        Skybox     = nullptr;
    };
} // namespace lucid::scene
