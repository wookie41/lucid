#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "renderable/mesh_renderable.hpp"

namespace lucid::gpu
{
    class CCubemap;
    struct FGPUState;
};

namespace lucid::scene
{
    class CLight;
    struct FRenderable;

    class CSkybox
    {
    public:
        explicit  CSkybox(gpu::CCubemap* InSkyboxCubemap) : SkyboxCubemap(InSkyboxCubemap) {}

        gpu::CCubemap* GetCubemap() const { return SkyboxCubemap; }
    
    private:
        gpu::CCubemap* SkyboxCubemap = nullptr;
    };

    CSkybox CreateSkybox(const FArray<FString>& InSkyboxFacesPaths, const FANSIString& InName);

    /*
    * The RenderScene contains things like objects to render, lights, fog voluems
    * in a Renderer-implementation-agnostic format. The specific Renderers then
    * use the provided scene information to render the scene in their own specific way
    */
    class CRenderScene
    {
    public:

        CRenderScene() = default;
        
        void AddStaticMesh(CStaticMesh* InStaticMesh);
        void AddLight(CLight* InLight);
        void SetSkybox(CSkybox* InSkybox);

        inline const FLinkedList<CStaticMesh>&  GetStaticMeshes() const { return StaticMeshes; }
        inline const FLinkedList<CLight>&       GetLights() const { return Lights; }
        inline CSkybox*                         GetSkybox() const { return SceneSkybox; }
    
    private:
        FLinkedList<CStaticMesh>    StaticMeshes;
        FLinkedList<CLight>         Lights;
        CSkybox*                    SceneSkybox = nullptr;
    };
} // namespace lucid::scene
