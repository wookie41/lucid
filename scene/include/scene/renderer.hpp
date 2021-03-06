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
        gpu::Framebuffer* Framebuffer = nullptr;
        const class Camera* Camera = nullptr;
        gpu::Viewport Viewport;
    };

    class Renderer
    {
      public:
        explicit Renderer(gpu::Shader* InDefaultShader) : DefaultShader(InDefaultShader) {}

        /**
        * Called before the first Render() call so that the renderer can set itself up, like create additional framebuffers and etc.
        * Can be called multiple time, e.x. when we want to adjust the renderer to a different RenderTarget, but subsequent Setup() calls
        * have be preceded by Cleanup() calls
        */
        virtual void Setup(const RenderTarget* InRenderTarget)
        {
            DefaultRenderTarget = InRenderTarget;
        };
        
        /**
         * Renders the scene into the specified target if provided or into the target set by Setup().
         */
        virtual void Render(const RenderScene* InSceneToRender, const RenderTarget* InRenderTarget = nullptr) = 0;

        /**
        * Called before the first renderer is deleted so it can cleanup whatever it did in Setup() or during Render(s)().
        */
        virtual void Cleanup() = 0;

        virtual ~Renderer() = default;

      protected:

        const RenderTarget* DefaultRenderTarget;
        gpu::Shader* DefaultShader;
    };

} // namespace lucid::scene
