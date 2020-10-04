#pragma once

#include "scene/renderer.hpp"

namespace lucid::scene
{
    class ForwardBlinnPhongRenderer : public Renderer
    {
      public:
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        ForwardBlinnPhongRenderer(const uint32_t& MaxNumOfDirectionalLights, gpu::Shader* DefaultShader)
        : Renderer(DefaultShader), maxNumOfDirectionalLights(MaxNumOfDirectionalLights)
        {
        }

        virtual void Render(const RenderScene* SceneToRender, const RenderTarget* Target) override;

        inline void SetAmbientStrength(const float& AmbientStrength)
        {
            ambientStrength = AmbientStrength;
        }

        virtual ~ForwardBlinnPhongRenderer() = default;

      private:
        inline void SetupFramebuffer(gpu::Framebuffer* Framebuffer);
        inline void SetupRendererWideUniforms(gpu::Shader* Shader, const RenderTarget* RenderTarget);
        inline void SetupLights(gpu::Shader* Shader, const RenderScene* Scene);

        float ambientStrength = 0.2;
        uint32_t maxNumOfDirectionalLights;
    };
} // namespace lucid::scene