#pragma once

#include <glm/vec2.hpp>

#include "material.hpp"
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
    class CPixelBuffer;
}; // namespace lucid::gpu

namespace lucid::scene
{
    constexpr int FRAME_DATA_BUFFERS_COUNT = 3;

    constexpr int PREPASS_DATA_BUFFER_SIZE  = 1024 * 1024;
    constexpr int INSTANCE_DATA_BUFFER_SIZE = 1024 * 1024;
    constexpr int ACTOR_DATA_BUFFER_SIZE    = 1024 * 1024;

#pragma pack(push, 1)
    struct FForwardPrepassUniforms
    {
        u32 bHasNormalMap                 = 0;
        u32 bHasDisplacementMap           = 0;
        u64 NormalMapBindlessHandle       = 0;
        u64 DisplacementMapBindlessHandle = 0;
    };
#pragma pack(pop)

    struct FFreeMaterialBufferEntries
    {
        EMaterialType    MaterialType;
        std::vector<u32> Indices;
        gpu::CFence* Fence; // when this fence is signaled, the buffers are re-written for FMaterialDataBuffer::FreeIndices so they can be recycled
    };

    struct FMaterialDataBuffer
    {
        gpu::CGPUBuffer* GPUBuffer    = nullptr;
        char*            MappedPtr    = nullptr;
        EMaterialType    MaterialType = EMaterialType::NONE;
        u32              NumEntries   = 0;
        u32              MaxEntries   = 0;
        std::vector<i32> FreeIndices;
    };

    struct FMeshBatch
    {
        gpu::CVertexArray*   MeshVertexArray    = nullptr;
        gpu::CShader*        Shader             = nullptr;
        u32                  BatchedSoFar       = 0; // Total number of batched meshes processed up until this batch
        FMaterialDataBuffer* MaterialDataBuffer = nullptr;
        u32                  BatchSize          = 0;

        std::vector<CMaterial*> BatchedMaterials; // this is currently needed only for the prepass and should be removed
    };

    class CForwardRenderer : public CRenderer
    {
      public:
        // Make sure that 'MaxNumOfDirectionalLights" matches the define in the shader
        CForwardRenderer(const u32& InMaxNumOfDirectionalLights, const u8& InNumSSAOSamples);

        virtual void Setup() override;
        virtual void Render(FRenderScene* InSceneToRender, const FRenderView* InRenderView) override;
        virtual void ResetState() override;
        virtual void Cleanup() override;

        virtual gpu::CFramebuffer* GetResultFramebuffer() override { return FrameResultFramebuffer; }
        virtual gpu::CTexture*     GetResultFrameTexture() override { return FrameResultTextures[(GRenderStats.FrameNumber - 2) % NumFrameBuffers]; }

        virtual ~CForwardRenderer() = default;

        /** Renderer properties, have to be set before the first Setup() call */
        float AmbientStrength     = 0.1;
        int   NumSamplesPCF       = 5;
        int   NumFrameBuffers     = 2;
        bool  bEnableSSAO         = true;
        u8    NumSSAOSamples      = 64;
        float SSAOBias            = 0.025;
        float SSAORadius          = 0.5;
        bool  bDrawGrid           = true;
        bool  bEnableDepthPrepass = true;

      private:
        FMaterialDataBuffer CreateMaterialBuffer(CMaterial const* InMaterial, const u32& InMaterialBufferSize);

        void FreeMaterialBufferEntry(const EMaterialType& InMaterialType, const i32& InIndex);
        void HandleMaterialBufferUpdateIfNecessary(CMaterial* Material);
        void CreateMeshBatches(FRenderScene* InSceneToRender);

        void SetupGlobalRenderData(const FRenderView* InRenderView);

        void GenerateShadowMaps(FRenderScene* InSceneToRender);
        void Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderSource);
        void LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderSource);

        inline void BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer);

        void        RenderStaticMeshes(const FRenderScene* InScene, const FRenderView* InRenderView);
        inline void RenderLightContribution(const CLight* InLight, const FRenderScene* InScene, const FRenderView* InRenderView);

        void RenderSkybox(const CSkybox* InSkybox, const FRenderView* InRenderView);

        void DoGammaCorrection(gpu::CTexture* InTexture);

#if DEVELOPMENT
        void DrawLightsBillboards(const FRenderScene* InScene, const FRenderView* InRenderView);
        void GenerateHitmap(const FRenderScene* InScene, const FRenderView* InRenderView);
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
        u64            SSAOBlurredBindlessHandle;

        gpu::CTexture**     LightingPassColorBuffers;
        gpu::CRenderbuffer* DepthStencilRenderBuffer;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::CTexture* CurrentFrameVSNormalMap;

        /** Generated in the depth prepass so we can later use it when calculating SSAO and things like that (VS - View Space) */
        gpu::CTexture* CurrentFrameVSPositionMap;

        /** Holds the final frame after post-processing, tone-mapping and gamma correction */
        gpu::CFramebuffer* FrameResultFramebuffer;
        gpu::CTexture**    FrameResultTextures;

        gpu::CGPUBuffer* GlobalDataUBO;
        char*            GlobalDataMappedPtr = nullptr;

        gpu::CGPUBuffer* PrepassDataSSBO;
        char*            PrepassDataMappedPtr = nullptr;

        gpu::CGPUBuffer* ActorDataSSBO;
        char*            ActorDataMappedPtr = nullptr;

        gpu::CGPUBuffer* InstanceDataSSBO;
        char*            InstanceDataMappedPtr = nullptr;

        std::vector<FMeshBatch>                                       MeshBatches;
        std::unordered_map<EMaterialType, FMaterialDataBuffer>        MaterialDataBufferPerMaterialType;
        std::vector<FFreeMaterialBufferEntries>                       FreeMaterialBuffersEntries;
        std::unordered_map<EMaterialType, FFreeMaterialBufferEntries> NewFreeMaterialBuffersEntries; // entries freed during current frame

        gpu::CFence* PersistentBuffersFences[FRAME_DATA_BUFFERS_COUNT];

        u64 BlankTextureBindlessHandle = 0;

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
        gpu::CPixelBuffer*  HitmapReadPixelBuffer;
        u8                  CurrentHitmapPBOIndex = 0;

        gpu::CTimer* FrameTimer = nullptr;

        gpu::CShader*      DebugLinesShader = nullptr;
        gpu::CVertexArray* DebugLinesVAO    = nullptr;
        gpu::CGPUBuffer*   DebugLinesVertexBuffers[FRAME_DATA_BUFFERS_COUNT]{ nullptr };
#endif
    };

}; // namespace lucid::scene