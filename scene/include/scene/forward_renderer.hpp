#pragma once

#include <glm/vec2.hpp>

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
    class Renderbuffer;
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
            gpu::Shader* InDefaultRenderableShader,
            gpu::Shader* InPrepassShader,
            gpu::Shader* InSSAOShader,
            gpu::Shader* SkyboxShader);

        virtual void Setup() override;
        virtual void Render(const RenderScene* InSceneToRender, const RenderSource* InRenderSource = nullptr) override;
        virtual void Cleanup() override;
        virtual gpu::Framebuffer* GetFinalFramebuffer() override { return LightingPassFramebuffer; }

        virtual ~ForwardRenderer() = default;

        /** Renderer properties, have to be set before the first Setup() call */
        float AmbientStrength = 0.1;
        int NumSamplesPCF = 5;
        glm::ivec2 FramebufferSize;

    private:

        void DepthPrepass(const RenderScene* InSceneToRender, const RenderSource* InRenderSource);
        void LightingPass(const RenderScene* InSceneToRender, const RenderSource* InRenderSource);
        
        inline void BindAndClearFramebuffer(gpu::Framebuffer* InFramebuffer);
        inline void SetupRendererWideUniforms(gpu::Shader* InShader, const RenderSource* InRenderSource);
        
        void Render(gpu::Shader* InShader, const Renderable* InRenderable);
        void RenderStaticGeometry(const RenderScene* InScene, const RenderSource* InRenderSource);
        void RenderWithoutLights(const RenderScene* InScene, const RenderSource* InRenderSource);
        void RenderLightContribution(const Light* InLight, const RenderScene* InScene, const RenderSource* InRenderSource);
        void SetupLight(gpu::Shader* InShader, const Light* InLight);

        void RenderSkybox(const Skybox* InSkybox, const RenderSource* InRenderSource);

        u32 MaxNumOfDirectionalLights;

        gpu::Shader* SkyboxShader;
        gpu::Shader* PrepassShader;
        gpu::Shader* SSAOShader;

        /** Framebuffer used for when doing the depth-only prepass */
        gpu::Framebuffer* PrepassFramebuffer;

        /** Framebuffer used when calculating SSAO */
        gpu::Framebuffer* SSAOFramebuffer;
        
        /** Framebuffer used for when doing the lighting pass */
        gpu::Framebuffer* LightingPassFramebuffer;

        /** Texture in which the result of SSAO algorithm will be stored */
        gpu::Texture* SSAOTexture;
        
        gpu::Texture* LightingPassColorBuffer;
        gpu::Renderbuffer* DepthStencilRenderBuffer;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::Texture* CurrentFrameVSNormalMap;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::Texture* CurrentFrameVSPositionMap;        
    };

}; // namespace lucid::scene