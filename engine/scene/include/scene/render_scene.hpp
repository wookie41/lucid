#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "renderable/mesh_renderable.hpp"

namespace lucid::gpu
{
    class CCubemap;
    struct FGPUState;
}; // namespace lucid::gpu

namespace lucid::scene
{
    class CLight;

    class CSkybox : IRenderable

    {
      public:
        explicit CSkybox(const u32& InId, const FDString& InName, const IRenderable* InParent, gpu::CCubemap* InSkyboxCubemap)
        : IRenderable(InId, InName, InParent), SkyboxCubemap(InSkyboxCubemap)
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
    class CRenderScene
    {
      public:
        CRenderScene() = default;

        void AddStaticMesh(CStaticMesh* InStaticMesh);
        void AddLight(CLight* InLight);
        void SetSkybox(CSkybox* InSkybox);

        inline const FLinkedList<CStaticMesh>& GetStaticMeshes() const { return StaticMeshes; }
        inline const FLinkedList<CLight>& GetLights() const { return Lights; }
        inline CSkybox* GetSkybox() const { return SceneSkybox; }

      private:
        FLinkedList<CStaticMesh> StaticMeshes;
        FLinkedList<CLight> Lights;
        CSkybox* SceneSkybox = nullptr;
    };
} // namespace lucid::scene
