#pragma once

#include "scene/renderer.hpp"

namespace lucid::scene
{
    class ForwardBlinnPhongRenderer : public Renderer
    {
      public:
        explicit ForwardBlinnPhongRenderer(gpu::Shader* DefaultShader) : Renderer(DefaultShader) {}

        virtual void Render(const RenderScene* SceneToRender, const RenderTarget* Target) override;

        inline void SetAmbientStrength(const float& AmbientStrength)
        {
            ambientStrength = AmbientStrength;
        }

        virtual ~ForwardBlinnPhongRenderer() = default;

      private:
        float ambientStrength = 0.1;
    };
} // namespace lucid::scene