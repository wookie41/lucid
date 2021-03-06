#pragma once

#include "scene/renderer.hpp"
#include "common/strings.hpp"

namespace lucid::resources
{
    class MeshResource;
};

namespace lucid::gpu
{
    class Shader;
    class Texture;
};

namespace lucid::scene
{
    // This renderer renders the geometry within the scene for each light in the scene
    // this mean that the same object might be rendered multiple times if it's affected
    // by multiple light.

    class ForwardRenderer : public Renderer
    {
      public:
        
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        ForwardRenderer(
            const u32& MaxNumOfDirectionalLights,
            gpu::Shader* InDefaultShader,
            gpu::Shader* InDepthPrepassShader,
            gpu::Shader* SkyboxShader);

        virtual void Setup(const RenderTarget* InRenderTarget) override;
        virtual void Render(const RenderScene* InSceneToRender, const RenderTarget* InRenderTarget = nullptr) override;
        virtual void Cleanup() override;

        virtual ~ForwardRenderer() = default;

        float AmbientStrength = 0.1;
        int NumSamplesPCF = 5;

    private:

        inline void BindAndClearFramebuffer(gpu::Framebuffer* Framebuffer);
        inline void SetupRendererWideUniforms(gpu::Shader* Shader, const RenderTarget* RenderTarget);
        
        void Render(gpu::Shader* Shader, Renderable* const ToRender);
        void RenderStaticGeometry(const RenderScene* InScene, const RenderTarget* InRenderTarget);
        void RenderWithoutLights(const RenderScene* InScene, const RenderTarget* InRenderTarget);
        void RenderLightContribution(const Light* InLight, const RenderScene* InScene, const RenderTarget* InRenderTarget);
        void SetupLight(gpu::Shader* Shader, const Light* InLight);

        void RenderSkybox(const Skybox* InSkybox, const RenderTarget* InRenderTarget);

        u32 MaxNumOfDirectionalLights;

        gpu::Shader* SkyboxShader;
        gpu::Shader* DepthPrePassShader;
    };

}; // namespace lucid::scene