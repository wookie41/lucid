#pragma once

#include <glm/vec2.hpp>

#include "scene/renderer.hpp"

namespace lucid::resources
{
    class CMeshResource;
};

namespace lucid::gpu
{
    class CShader;
    class CTexture;
    class CRenderbuffer;
}; // namespace lucid::gpu

namespace lucid::scene
{
    // This renderer renders the geometry within the scene for each light in the scene
    // this mean that the same object might be rendered multiple times if it's affected
    // by multiple light.

    class ForwardRenderer : public CRenderer
    {
      public:
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        ForwardRenderer(const u32& InMaxNumOfDirectionalLights,
                        const u8 InNumSSAOSamples,
                        gpu::CShader* InDefaultRenderableShader,
                        gpu::CShader* InPrepassShader,
                        gpu::CShader* InSSAOShader,
                        gpu::CShader* InSimpleBlurShader,
                        gpu::CShader* SkyboxShader);

        virtual void Setup() override;
        virtual void Render(const FRenderScene* InSceneToRender, const FRenderView* InRenderView) override;
        virtual void Cleanup() override;

        virtual gpu::CFramebuffer* GetResultFramebuffer() override { return LightingPassFramebuffer; }

        virtual ~ForwardRenderer() = default;

        /** Renderer properties, have to be set before the first Setup() call */
        float AmbientStrength = 0.1;
        int NumSamplesPCF = 5;
        glm::ivec2 FramebufferSize;

      private:
        void Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderSource);
        void LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderSource);

        inline void BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer);
        inline void SetupRendererWideUniforms(gpu::CShader* InShader, const FRenderView* InRenderView);

        void Render(gpu::CShader* InShader, const FRenderable* InRenderable);
        void RenderStaticGeometry(const FRenderScene* InScene, const FRenderView* InRenderView);
        void RenderWithoutLights(const FRenderScene* InScene, const FRenderView* InRenderView);
        void RenderLightContribution(const CLight* InLight, const FRenderScene* InScene, const FRenderView* InRenderView);
        void SetupLight(gpu::CShader* InShader, const CLight* InLight);

        void RenderSkybox(const FSkybox* InSkybox, const FRenderView* InRenderView);

        u32 MaxNumOfDirectionalLights;

        /** Skybox Shader */
        gpu::CShader* SkyboxShader;

        /** Prepass Shader */
        gpu::CShader* PrepassShader;

        /** SSAO Shader */
        gpu::CShader* SSAOShader;

        u8 NumSSAOSamples = 64;
        float SSAOBias = 0.025;
        float SSAORadius = 0.5;

        /** Blur shader */
        gpu::CShader* SimpleBlurShader;

        u8 SimpleBlurXOffset = 2;
        u8 SimpleBlurYOffset = 2;
        
        /** Framebuffer used for when doing the depth-only prepass */
        gpu::CFramebuffer* PrepassFramebuffer;

        /** Framebuffer used when calculating SSAO */
        gpu::CFramebuffer* SSAOFramebuffer;

        /** Framebuffer used for blur calculation */
        gpu::CFramebuffer* BlurFramebuffer;

        /** Framebuffer used for when doing the lighting pass */
        gpu::CFramebuffer* LightingPassFramebuffer;

        /** Texture holding random rotation vectors for calculating SSAO */
        gpu::CTexture* SSAONoise;

        /** Texture in which the result of SSAO algorithm will be stored */
        gpu::CTexture* SSAOResult;

        /** Texture in which the result of blurring the SSAOResult texture will be stored */
        gpu::CTexture* SSAOBlurred;

        gpu::CTexture* LightingPassColorBuffer;
        gpu::CRenderbuffer* DepthStencilRenderBuffer;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::CTexture* CurrentFrameVSNormalMap;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::CTexture* CurrentFrameVSPositionMap;
    };

}; // namespace lucid::scene