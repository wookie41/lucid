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
    class Camera;
    
    struct RenderSource
    {
        Camera* Camera;
        gpu::Viewport Viewport;
    };

    class Renderer
    {
      public:
        explicit Renderer(gpu::Shader* InDefaultShader) : DefaultShader(InDefaultShader) {}

        /**
          * Called before the first Render() call so that the renderer can set itself up, like create additional framebuffers and etc.
          * Can be called multiple time, subsequent Setup() calls have be preceded by Cleanup() calls
        */
        virtual void Setup() = 0;
        
        /**
         * Renders the scene from the specified source
         */
        virtual void Render(const RenderScene* InSceneToRender, const RenderSource* InRenderSource) = 0;

        /**
         * Called before the first renderer is deleted so it can cleanup whatever it did in Setup() or during Render(s)().
         */
        virtual void Cleanup() = 0;

        /**
        * Returns the framebuffer which holds the result of last Render() call.
        */
        virtual gpu::Framebuffer* GetFinalFramebuffer() = 0;

        
        virtual ~Renderer() = default;

      protected:

        gpu::Shader* DefaultShader;
    };

} // namespace lucid::scene
