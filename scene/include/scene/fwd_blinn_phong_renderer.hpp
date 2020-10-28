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
    class ForwardBlinnPhongRenderer : public Renderer
    {
      public:
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        ForwardBlinnPhongRenderer(const uint32_t& MaxNumOfDirectionalLights, gpu::Shader* DefaultShader);

        virtual void Render(const RenderScene* SceneToRender, const RenderTarget* Target) override;

        inline void SetAmbientStrength(const float& AmbientStrength) { ambientStrength = AmbientStrength; }

        virtual ~ForwardBlinnPhongRenderer() = default;

      private:

        void RenderStaticGeometry(const RenderScene* SceneToRender, const RenderTarget* Target);
        void Render(gpu::Shader* Shader, Renderable* const ToRender);
        void SetupDirectionalLight(gpu::Shader* Shader, const DirectionalLight* Light);

        inline void SetupFramebuffer(gpu::Framebuffer* Framebuffer);
        inline void SetupRendererWideUniforms(gpu::Shader* Shader, const RenderTarget* RenderTarget);

        float ambientStrength = 0.2;
        uint32_t maxNumOfDirectionalLights;
    };

    Renderable* CreateBlinnPhongRenderable(DString MeshName, resources::MeshResource* Mesh, gpu::Shader* CustomShader = nullptr);

}; // namespace lucid::scene