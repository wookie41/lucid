#pragma once

#include "devices/gpu/viewport.hpp"
#include "scene/render_scene.hpp"

namespace lucid::gpu
{
    class Framebuffer;
    class Shader;
}; // namespace lucid::gpu

namespace lucid::scene
{
    struct RenderTarget
    {
        gpu::Framebuffer* Framebuffer = nullptr; // null represents the defautl framebuffer
        const class Camera* Camera = nullptr;
        gpu::Viewport Viewport;
    };

    class Renderer
    {
      public:
        explicit Renderer(gpu::Shader* DefaultShader) : defaultShader(DefaultShader) {}

        virtual void Render(RenderScene* const SceneToRender, RenderTarget* const Target) = 0;

        virtual ~Renderer() = default;

      protected:
        gpu::Shader* defaultShader;
    };
} // namespace lucid::scene
