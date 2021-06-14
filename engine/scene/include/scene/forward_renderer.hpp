#pragma once

#include <glm/vec2.hpp>

#include "devices/gpu/gpu.hpp"
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
    class CTimer;
    class CFence;
}; // namespace lucid::gpu

namespace lucid::scene
{
    // This renderer renders the geometry within the scene for each light in the scene
    // this mean that the same object might be rendered multiple times if it's affected
    // by multiple light.

    constexpr int DEBUG_LINES_BUFFERS_COUNT          = 3;
    constexpr int PREPASS_DATA_BUFFERS_COUNT         = 3;
    constexpr int MATERIAL_DATA_BUFFERS_COUNT        = 3;
    constexpr int COMMON_INSTANCE_DATA_BUFFERS_COUNT = 3;
    constexpr int BINDLESS_TEXTURES_BUFFERS_COUNT    = 3;

    constexpr int PREPASS_DATA_BUFFER_SIZE            = 2048;
    constexpr int MATERIAL_DATA_BUFFER_SIZE           = 8192;
    constexpr int COMMON_INSTANCE_DATA_BUFFER_SIZE    = 4096;
    constexpr int BINDLESS_TEXTURES_ARRAY_BUFFER_SIZE = 1024;

#pragma pack(push, 1)
    struct FForwardPrepassUniforms
    {
        u32 bHasNormalMap                 = 0;
        u32 bHasDisplacementMap           = 0;
        u64 NormalMapBindlessHandle       = 0;
        u64 DisplacementMapBindlessHandle = 0;
    };
#pragma pack(pop)

    struct FBatchedMesh
    {
        int        NormalMultiplier = 1;
        CMaterial* Material         = nullptr;
    };

    struct FMeshBatch
    {
        gpu::CVertexArray*        MeshVertexArray;
        gpu::CShader*             Shader;
        std::vector<FBatchedMesh> BatchedMeshes;
        u32                       BatchedSoFar; // Total number of batched meshes processed up until this batch
        u32                       MaterialDataOffset; // Offset at which material data for this batch start
        u32                       MaterialDataSize; // Size of material data for this batch
    };

    class CForwardRenderer : public CRenderer
    {
      public:
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        CForwardRenderer(const u32& InMaxNumOfDirectionalLights, const u8& InNumSSAOSamples);

        virtual void Setup() override;
        virtual void Render(FRenderScene* InSceneToRender, const FRenderView* InRenderView) override;
        virtual void Cleanup() override;

        virtual gpu::CFramebuffer* GetResultFramebuffer() override { return FrameResultFramebuffer; }

        virtual ~CForwardRenderer() = default;

        /** Renderer properties, have to be set before the first Setup() call */
        float AmbientStrength = 0.1;
        int   NumSamplesPCF   = 5;

      private:
        void CreateMeshBatches(FRenderScene* InSceneToRender);

        void GenerateShadowMaps(FRenderScene* InSceneToRender);
        void Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderSource);
        void LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderSource);

        inline void BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer);
        inline void SetupRendererWideUniforms(gpu::CShader* InShader, const FRenderView* InRenderView);

        void        RenderStaticMeshes(const FRenderScene* InScene, const FRenderView* InRenderView);
        inline void RenderLightContribution(const CLight* InLight, const FRenderScene* InScene, const FRenderView* InRenderView);
        void        RenderStaticMesh(gpu::CShader**            LastShader,
                                     const scene::CStaticMesh* InStaticMesh,
                                     const scene::CLight*      InLight,
                                     const FRenderView*        InRenderView);

        void RenderSkybox(const CSkybox* InSkybox, const FRenderView* InRenderView);

        void DoGammaCorrection(gpu::CTexture* InTexture);

#if DEVELOPMENT
        void DrawLightsBillboards(const FRenderScene* InScene, const FRenderView* InRenderView);
        void GenerateHitmap(const FRenderScene* InScene, const FRenderView* InRenderView) const;
        void RenderWithDefaultMaterial(const resources::CMeshResource* InMeshResource,
                                       const u16&                      InSubMeshIndex,
                                       const CLight*                   InLight,
                                       const FRenderView*              InRenderView,
                                       const glm::mat4&                InModelMatrix);
        void RenderWorldGrid(const FRenderView* InRenderView);
        void RenderDebugLines(const FRenderView* InRenderView);
#endif

        u32   MaxNumOfDirectionalLights;
        float Gamma = 2.2;

        gpu::CShader* FlatShader;

        /** VAO used when doing post-processing */
        gpu::CVertexArray* ScreenWideQuadVAO = nullptr;

        /** VAO used when rendering skybox */
        gpu::CVertexArray* UnitCubeVAO = nullptr;

        /** Preconfigured pipeline states */
        gpu::FPipelineState ShadowMapGenerationPipelineState;
        gpu::FPipelineState PrepassPipelineState;
        gpu::FPipelineState InitialLightLightpassPipelineState;
        gpu::FPipelineState LightpassPipelineState;
        gpu::FPipelineState SkyboxPipelineState;
        gpu::FPipelineState GammaCorrectionPipelineState;

        gpu::CShader* ShadowMapShader;
        gpu::CShader* ShadowCubeMapShader;
        gpu::CShader* SkyboxShader;
        gpu::CShader* PrepassShader;
        gpu::CShader* SSAOShader;
        gpu::CShader* BillboardShader;
        gpu::CShader* GammaCorrectionShader;

        u8    NumSSAOSamples = 64;
        float SSAOBias       = 0.025;
        float SSAORadius     = 0.5;

        /** Blur shader */
        gpu::CShader* SimpleBlurShader;

        u8 SimpleBlurXOffset = 2;
        u8 SimpleBlurYOffset = 2;

        /** Framebuffer when generating shadow maps*/
        gpu::CFramebuffer* ShadowMapFramebuffer;

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

        gpu::CTexture*      LightingPassColorBuffer;
        gpu::CRenderbuffer* DepthStencilRenderBuffer;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::CTexture* CurrentFrameVSNormalMap;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::CTexture* CurrentFrameVSPositionMap;

        /** Holds the final frame after post-processing, tone-mapping and gamma correction */
        gpu::CFramebuffer* FrameResultFramebuffer;
        gpu::CTexture*     FrameResultTexture;

        gpu::CGPUBuffer* PrepassDataSSBOBuffer;
        gpu::CFence*     PrepassDataBufferFences[PREPASS_DATA_BUFFERS_COUNT];

        gpu::CGPUBuffer* CommonInstanceDataSSBO;
        gpu::CFence*     CommonInstanceDataBufferFences[MATERIAL_DATA_BUFFERS_COUNT];

        gpu::CGPUBuffer* MaterialDataSSBOBuffer;
        gpu::CFence*     MaterialDataBufferFences[MATERIAL_DATA_BUFFERS_COUNT];

        u32  NumBindlessTexturesUsed  = 0;
        u64* BindlessTexturesArrayPtr = nullptr;

        std::vector<FMeshBatch> MeshBatches;

#if DEVELOPMENT
      public:
        void UIDrawSettingsWindow() override;

        glm::vec2 BillboardViewportSize{ 0.1, 0.15 };

        /**
         *  Shader that saves ids of the objects in the scene to a texture
         *  so it can be later used for picking, used only for tools
         */
        gpu::CShader* HitMapShader          = nullptr;
        gpu::CShader* BillboardHitMapShader = nullptr;
        gpu::CShader* WorldGridShader       = nullptr;

      private:
        gpu::FPipelineState LightsBillboardsPipelineState;
        gpu::FPipelineState WorldGridPipelineState;
        gpu::FPipelineState DebugLinesPipelineState;

        /** Used to render ids of the objects in the scene so we can do nice mouse picking in the tools */
        gpu::CTexture*      HitMapTexture;
        gpu::CFramebuffer*  HitMapFramebuffer;
        gpu::FPipelineState HitMapGenerationPipelineState;
        gpu::CRenderbuffer* HitMapDepthStencilRenderbuffer;

        gpu::CTimer* FrameTimer = nullptr;

        gpu::CShader*      DebugLinesShader = nullptr;
        gpu::CVertexArray* DebugLinesVAO    = nullptr;
        gpu::CGPUBuffer*   DebugLinesVertexBuffers[DEBUG_LINES_BUFFERS_COUNT]{ nullptr };
        gpu::CFence*       DebugLinesFences[DEBUG_LINES_BUFFERS_COUNT]{ nullptr };
#endif
    };

}; // namespace lucid::scene