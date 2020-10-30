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
};

namespace lucid::scene
{
    // This renderer renders the geometry within the scene for each light in the scene
    // this mean that the same object might be rendered multiple times if it's affected
    // by multiple light.

    class ForwardBlinnPhongRenderer : public Renderer
    {
      public:
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        ForwardBlinnPhongRenderer(const uint32_t& MaxNumOfDirectionalLights, gpu::Shader* DefaultShader);

        virtual void Render(RenderScene* const  SceneToRender, RenderTarget* const Target) override;

        inline void SetAmbientStrength(const float& AmbientStrength) { ambientStrength = AmbientStrength; }

        virtual ~ForwardBlinnPhongRenderer() = default;

      private:

        void Render(gpu::Shader* Shader, Renderable* const ToRender);

        void RenderStaticGeometry(RenderScene* const SceneToRender, RenderTarget* const Target);

        void RenderWithoutLights(RenderScene* const SceneToRender, RenderTarget* const Target);

        void RenderLightContribution(Light* const InLight, RenderScene* const SceneToRender, RenderTarget* const Target);
        void SetupLight(gpu::Shader* Shader, Light* const InLight); 

        inline void SetupFramebuffer(gpu::Framebuffer* Framebuffer);
        inline void SetupRendererWideUniforms(gpu::Shader* Shader, const RenderTarget* RenderTarget);

        float ambientStrength = 0.2;
        uint32_t maxNumOfDirectionalLights;
    };

    Renderable* CreateBlinnPhongRenderable(DString MeshName, resources::MeshResource* Mesh, gpu::Shader* CustomShader = nullptr);

}; // namespace lucid::scene