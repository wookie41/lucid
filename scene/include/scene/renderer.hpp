#pragma once

#include "devices/gpu/viewport.hpp"
#include "scene/render_scene.hpp"

namespace lucid::gpu
{
    class CFramebuffer;
    class CShader;
}; // namespace lucid::gpu

namespace lucid::scene
{
    class CCamera;
    
    struct FRenderView
    {
        CCamera* Camera;
        gpu::FViewport Viewport;
    };

    class CRenderer
    {
      public:
        explicit CRenderer(gpu::CShader* InDefaultShader) : DefaultRenderableShader(InDefaultShader) {}

        /**
          * Called before the first Render() call so that the renderer can set itself up, like create additional framebuffers and etc.
          * Can be called multiple time, subsequent Setup() calls have be preceded by Cleanup() calls
        */
        virtual void Setup() = 0;
        
        /**
         * Renders the scene from the specified view
         */
        virtual void Render(const FRenderScene* InSceneToRender, const FRenderView* InRenderView) = 0;

        /**
         * Called before the renderer is deleted so it can cleanup whatever it did in Setup() or during Render(s)().
         */
        virtual void Cleanup() = 0;

        /**
        * Returns the framebuffer which holds the result of last Render() call.
        */
        virtual gpu::CFramebuffer* GetResultFramebuffer() = 0;

        
        virtual ~CRenderer() = default;

      protected:

        gpu::CShader* DefaultRenderableShader;
    };

} // namespace lucid::scene
