#include "scene/forward_renderer.hpp"

#include <set>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "devices/gpu/shaders_manager.hpp"
#include "engine/engine.hpp"

#include "common/log.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/viewport.hpp"
#include "devices/gpu/cubemap.hpp"
#include "devices/gpu/timer.hpp"
#include "devices/gpu/fence.hpp"
#include "devices/gpu/pixelbuffer.hpp"

#include "scene/actors/lights.hpp"
#include "scene/camera.hpp"
#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"
#include "scene/actors/terrain.hpp"

#include "misc/basic_shapes.hpp"
#include "misc/math.hpp"

#include "resources/resources_holder.hpp"
#include "resources/mesh_resource.hpp"

namespace lucid::scene
{
    static const u8       NO_LIGHT = 0;
    static const FSString LIGHT_TYPE("uLightType");

    static const FSString VIEWPORT_SIZE("uViewportSize");

    static const FSString SSAO_POSITIONS_VS("uPositionsVS");
    static const FSString SSAO_NORMALS_VS("uNormalsVS");
    static const FSString SSAO_NOISE("uNoise");
    static const FSString SSAO_NOISE_SCALE("uNoiseScale");
    static const FSString SSAO_RADIUS("uRadius");
    static const FSString SSAO_BIAS("uBias");

    static const FSString SIMPLE_BLUR_OFFSET_X("uOffsetX");
    static const FSString SIMPLE_BLUR_OFFSET_Y("uOffsetY");
    static const FSString SIMPLE_BLUR_TEXTURE("uTextureToBlur");

    static const FSString BILLBOARD_MATRIX("uBillboardMatrix");
    static const FSString BILLBOARD_VIEWPORT_SIZE("uBillboardViewportSize");
    static const FSString BILLBOARD_TEXTURE("uBillboardTexture");
    static const FSString BILLBOARD_WORLD_POS("uBillboardWorldPos");
    static const FSString BILLBOARD_COLOR_TINT("uBillboardColorTint");

    static const FSString MODEL_MATRIX("uModel");
    static const FSString VIEW_MATRIX("uView");
    static const FSString PROJECTION_MATRIX("uProjection");

    static const FSString SKYBOX_CUBEMAP("uSkybox");

    static const FSString GAMMA("uGamma");
    static const FSString SCENE_TEXTURE("uSceneTexture");

    static const FSString MESH_BATCH_OFFSET("uMeshBatchOffset");

    constexpr gpu::EImmutableBufferUsage COHERENT_WRITE_USAGE =
      (gpu::EImmutableBufferUsage)(gpu::EImmutableBufferUsage::IMM_BUFFER_WRITE | gpu::EImmutableBufferUsage::IMM_BUFFER_COHERENT);

    constexpr gpu::EBufferMapPolicy COHERENT_WRITE =
      (gpu::EBufferMapPolicy)(gpu::EBufferMapPolicy::BUFFER_WRITE | gpu::EBufferMapPolicy::BUFFER_COHERENT | gpu::EBufferMapPolicy::BUFFER_PERSISTENT);

    constexpr gpu::EGPUBuffer COLOR_AND_DEPTH = (gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH);

#if DEVELOPMENT
    static const FSString ACTOR_ID("uActorId");
#endif

    static constexpr u32 INITIAL_MATERIAL_DATA_BUFFER_SIZE = 1024 * 128 * 3; // 128 KiB

    /* Buffers used when preparing data to be memcpy'ed to the GPU */
    static char STAGING_BUFFER_TINY_0[1024];
    static char STAGING_BUFFER_SMALL_0[1024 * 16];
    static char STAGING_BUFFER_SMALL_1[1024 * 16];
    static char STAGING_BUFFER_MEDIUM_0[1024 * 1024 * 5];

#pragma pack(push, 1)

    struct FActorData
    {
        glm::mat4 ModelMatrix;
        i32       NormalMultiplier;
        u32       ActorId;
        char      _padding[8];
    };

    struct FInstanceData
    {
        u32 ActorDataIdx;
        u32 MaterialDataIdx;
    };

    struct FGlobalRenderData
    {
        glm::mat4 ProjectionMatrix;
        glm::mat4 ViewMatrix;
        glm::vec3 ViewPos;
        float     AmbientStrength;
        glm::vec2 ViewportSize;
        u64       AmbientOcclusionBindlessHandle;
        u32       NumPCFsamples;
        float     ParallaxHeightScale;
        float     NearPlane;
        float     FarPlane;
    };

#pragma pack(pop)

#define GLOBAL_DATA_BUFFER_SIZE                                                                       \
    (sizeof(FGlobalRenderData) + (sizeof(FGlobalRenderData) > gpu::GGPUInfo.UniformBlockAlignment ?   \
                                    sizeof(FGlobalRenderData) % gpu::GGPUInfo.UniformBlockAlignment : \
                                    gpu::GGPUInfo.UniformBlockAlignment % sizeof(FGlobalRenderData)))

    CForwardRenderer::CForwardRenderer(const u32& InMaxNumOfDirectionalLights, const u8& InNumSSAOSamples)
    : MaxNumOfDirectionalLights(InMaxNumOfDirectionalLights), NumSSAOSamples(InNumSSAOSamples)
    {
    }

    void CForwardRenderer::Setup()
    {
        if (ScreenWideQuadVAO == nullptr)
        {
            ScreenWideQuadVAO = misc::CreateQuadVAO();
        }

        if (UnitCubeVAO == nullptr)
        {
            UnitCubeVAO = misc::CreateCubeVAO();
        }

        ShadowMapShader       = GEngine.GetShadersManager().GetShaderByName("ShadowMap");
        ShadowCubeMapShader   = GEngine.GetShadersManager().GetShaderByName("ShadowCubemap");
        PrepassShader         = GEngine.GetShadersManager().GetShaderByName("ForwardPrepass");
        SSAOShader            = GEngine.GetShadersManager().GetShaderByName("SSAO");
        SimpleBlurShader      = GEngine.GetShadersManager().GetShaderByName("SimpleBlur");
        SkyboxShader          = GEngine.GetShadersManager().GetShaderByName("Skybox");
        BillboardShader       = GEngine.GetShadersManager().GetShaderByName("Billboard");
        FlatShader            = GEngine.GetShadersManager().GetShaderByName("Flat");
        GammaCorrectionShader = GEngine.GetShadersManager().GetShaderByName("GammaCorrection");

#if DEVELOPMENT
        EditorHelpersShader    = GEngine.GetShadersManager().GetShaderByName("Hitmap");
        EditorBillboardsShader = GEngine.GetShadersManager().GetShaderByName("BillboardHitmap");
        WorldGridShader        = GEngine.GetShadersManager().GetShaderByName("WorldGrid");
        DebugLinesShader       = GEngine.GetShadersManager().GetShaderByName("DebugLines");
#endif

        // Prepare pipeline states
        ShadowMapGenerationPipelineState.ClearColorBufferColor    = FColor{ 1 };
        ShadowMapGenerationPipelineState.ClearDepthBufferValue    = 1;
        ShadowMapGenerationPipelineState.IsDepthTestEnabled       = true;
        ShadowMapGenerationPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        ShadowMapGenerationPipelineState.IsBlendingEnabled        = false;
        ShadowMapGenerationPipelineState.IsCullingEnabled         = false;
        ShadowMapGenerationPipelineState.IsSRGBFramebufferEnabled = false;
        ShadowMapGenerationPipelineState.IsDepthBufferReadOnly    = false;

        PrepassPipelineState.ClearColorBufferColor = FColor{ 0 };
        PrepassPipelineState.IsDepthTestEnabled    = true;
        PrepassPipelineState.DepthTestFunction     = gpu::EDepthTestFunction::LEQUAL;
        PrepassPipelineState.IsBlendingEnabled     = false;
        PrepassPipelineState.IsCullingEnabled      = false;
        // @TODO
        // PrepassPipelineState.IsCullingEnabled = false;
        // PrepassPipelineState.CullMode = gpu::ECullMode::BACK;
        PrepassPipelineState.IsSRGBFramebufferEnabled = false;
        PrepassPipelineState.IsDepthBufferReadOnly    = false;

        LightpassPipelineState.ClearColorBufferColor = FColor{ 0 };
        LightpassPipelineState.IsDepthTestEnabled    = true;
        LightpassPipelineState.DepthTestFunction     = gpu::EDepthTestFunction::EQUAL;
        LightpassPipelineState.IsBlendingEnabled     = true;
        LightpassPipelineState.BlendFunctionSrc      = gpu::EBlendFunction::ONE;
        LightpassPipelineState.BlendFunctionDst      = gpu::EBlendFunction::ONE;
        LightpassPipelineState.BlendFunctionAlphaSrc = gpu::EBlendFunction::ONE;
        LightpassPipelineState.BlendFunctionAlphaDst = gpu::EBlendFunction::ONE;
        // @TODO
        LightpassPipelineState.IsCullingEnabled = false;
        // LightpassPipelineState.IsCullingEnabled = true;
        // LightpassPipelineState.CullMode = gpu::ECullMode::BACK;
        LightpassPipelineState.IsSRGBFramebufferEnabled = false;
        LightpassPipelineState.IsDepthBufferReadOnly    = true;
        LightpassPipelineState.Viewport.X               = 0;
        LightpassPipelineState.Viewport.Y               = 0;
        LightpassPipelineState.Viewport.Width           = ResultResolution.x;
        LightpassPipelineState.Viewport.Height          = ResultResolution.y;

        SkyboxPipelineState                          = LightpassPipelineState;
        SkyboxPipelineState.IsBlendingEnabled        = false;
        SkyboxPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        SkyboxPipelineState.IsSRGBFramebufferEnabled = false;

        GammaCorrectionPipelineState.ClearColorBufferColor    = FColor{ 0 };
        GammaCorrectionPipelineState.IsDepthTestEnabled       = false;
        GammaCorrectionPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        GammaCorrectionPipelineState.IsBlendingEnabled        = false;
        GammaCorrectionPipelineState.IsCullingEnabled         = false;
        GammaCorrectionPipelineState.IsSRGBFramebufferEnabled = false;
        GammaCorrectionPipelineState.IsDepthBufferReadOnly    = true;
        GammaCorrectionPipelineState.Viewport                 = LightpassPipelineState.Viewport;

#if DEVELOPMENT
        EditorHelpersPipelineState                          = SkyboxPipelineState;
        EditorHelpersPipelineState.IsDepthBufferReadOnly    = false;
        EditorHelpersPipelineState.IsSRGBFramebufferEnabled = false;

        WorldGridPipelineState.IsDepthTestEnabled       = true;
        WorldGridPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        WorldGridPipelineState.IsBlendingEnabled        = true;
        WorldGridPipelineState.IsCullingEnabled         = false;
        WorldGridPipelineState.IsSRGBFramebufferEnabled = false;
        WorldGridPipelineState.IsDepthBufferReadOnly    = true;
        WorldGridPipelineState.BlendFunctionSrc         = gpu::EBlendFunction::SRC_ALPHA;
        WorldGridPipelineState.BlendFunctionDst         = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;
        WorldGridPipelineState.BlendFunctionAlphaSrc    = gpu::EBlendFunction::SRC_ALPHA;
        WorldGridPipelineState.BlendFunctionAlphaDst    = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;
        WorldGridPipelineState.Viewport                 = LightpassPipelineState.Viewport;
#endif

        // Create the framebuffers
        ShadowMapFramebuffer    = gpu::CreateFramebuffer(FSString{ "ShadowmapFramebuffer" });
        PrepassFramebuffer      = gpu::CreateFramebuffer(FSString{ "PrepassFramebuffer" });
        LightingPassFramebuffer = gpu::CreateFramebuffer(FSString{ "LightingPassFramebuffer" });
        SSAOFramebuffer         = gpu::CreateFramebuffer(FSString{ "SSAOFramebuffer" });
        BlurFramebuffer         = gpu::CreateFramebuffer(FSString{ "BlueFramebuffer" });
        FrameResultFramebuffer  = gpu::CreateFramebuffer(FSString{ "FameResultFramebuffer" });

        // Create render targets in which we'll store some additional information during the depth prepass
        CurrentFrameVSNormalMap   = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                            ResultResolution.y,
                                                            gpu::ETextureDataType::FLOAT,
                                                            gpu::ETextureDataFormat::RGB16F,
                                                            gpu::ETexturePixelFormat::RGB,
                                                            0,
                                                            FSString{ "CurrentFrameVSNormalMap" });
        CurrentFrameVSPositionMap = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                              ResultResolution.y,
                                                              gpu::ETextureDataType::FLOAT,
                                                              gpu::ETextureDataFormat::RGB16F,
                                                              gpu::ETexturePixelFormat::RGB,
                                                              0,
                                                              FSString{ "CurrentFrameVSPositionMap" });

        // Create color attachment for the lighting pass framebuffer and a shared depth-setencil attachment
        DepthStencilRenderBuffer =
          gpu::CreateRenderbuffer(gpu::ERenderbufferFormat::DEPTH24_STENCIL8, ResultResolution, FSString{ "LightingPassRenderbuffer" });
        DepthStencilRenderBuffer->Bind();

        // Attach it to the lighting pass framebuffer
        LightingPassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        LightingPassFramebuffer->SetupDepthStencilAttachment(DepthStencilRenderBuffer);

        LightingPassColorBuffers = new gpu::CTexture*[NumFrameBuffers];
        FrameResultTextures      = new gpu::CTexture*[NumFrameBuffers];

        for (int i = 0; i < NumFrameBuffers; ++i)
        {
            LightingPassColorBuffers[i] = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                                    ResultResolution.y,
                                                                    gpu::ETextureDataType::FLOAT,
                                                                    gpu::ETextureDataFormat::RGBA,
                                                                    gpu::ETexturePixelFormat::RGBA,
                                                                    0,
                                                                    FSString{ "LightingPassColorBuffer" });

            LightingPassColorBuffers[i]->Bind();
            LightingPassColorBuffers[i]->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
            LightingPassColorBuffers[i]->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

            // Create textures which will hold the final result
            FrameResultTextures[i] = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                               ResultResolution.y,
                                                               gpu::ETextureDataType::FLOAT,
                                                               gpu::ETextureDataFormat::RGBA,
                                                               gpu::ETexturePixelFormat::RGBA,
                                                               0,
                                                               FSString{ "FrameResult" });
            FrameResultTextures[i]->Bind();
            FrameResultTextures[i]->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
            FrameResultTextures[i]->SetMagFilter(gpu::EMagTextureFilter::NEAREST);
        }

        // Setup the prepass framebuffer
        PrepassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        CurrentFrameVSNormalMap->Bind();
        CurrentFrameVSNormalMap->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        CurrentFrameVSNormalMap->SetMagFilter(gpu::EMagTextureFilter::NEAREST);
        PrepassFramebuffer->SetupColorAttachment(0, CurrentFrameVSNormalMap);

        CurrentFrameVSPositionMap->Bind();
        CurrentFrameVSPositionMap->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        CurrentFrameVSPositionMap->SetMagFilter(gpu::EMagTextureFilter::NEAREST);
        PrepassFramebuffer->SetupColorAttachment(1, CurrentFrameVSPositionMap);

        DepthStencilRenderBuffer->Bind();
        PrepassFramebuffer->SetupDepthStencilAttachment(DepthStencilRenderBuffer);

        // Create texture to store SSO result
        SSAOResult = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                               ResultResolution.y,
                                               gpu::ETextureDataType::FLOAT,
                                               gpu::ETextureDataFormat::R,
                                               gpu::ETexturePixelFormat::RED,
                                               0,
                                               FSString{ "SSAOResult" });
        SSAOResult->Bind();
        SSAOResult->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        SSAOResult->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

        // Setup a SSAO framebuffer
        SSAOFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        SSAOFramebuffer->SetupColorAttachment(0, SSAOResult);

        if (!SSAOFramebuffer->IsComplete())
        {
            LUCID_LOG(ELogLevel::ERR, LUCID_TEXT("Failed to setup the SSAO framebuffer"));
            return;
        }

        // Create texture for the blurred SSAO result
        SSAOBlurred = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                ResultResolution.y,
                                                gpu::ETextureDataType::FLOAT,
                                                gpu::ETextureDataFormat::R,
                                                gpu::ETexturePixelFormat::RED,
                                                0,
                                                FSString{ "SSOBlurred" });
        SSAOBlurred->Bind();
        SSAOBlurred->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        SSAOBlurred->SetMagFilter(gpu::EMagTextureFilter::NEAREST);
        SSAOBlurred->SetWrapSFilter(gpu::EWrapTextureFilter::CLAMP_TO_BORDER);
        SSAOBlurred->SetWrapTFilter(gpu::EWrapTextureFilter::CLAMP_TO_BORDER);
        SSAOBlurred->SetBorderColor({ 1, 1, 1, 1 });
        SSAOBlurredBindlessHandle = SSAOBlurred->GetBindlessHandle();
        SSAOBlurred->MakeBindlessResident();

        // Setup the SSAO shader
        SSAOShader->Use();
        SSAOShader->UseTexture(SSAO_POSITIONS_VS, CurrentFrameVSPositionMap);
        SSAOShader->UseTexture(SSAO_NORMALS_VS, CurrentFrameVSNormalMap);

        // Sample vectors
        for (int i = 0; i < NumSSAOSamples; ++i)
        {
            glm::vec3 Sample = math::RandomVec3();

            // Transform x and y to [-1, 1], keep z [0, 1] so the we sample around a hemisphere
            Sample.x = Sample.x * 2.0 - 1.0;
            Sample.y = Sample.y * 2.0 - 1.0;
            Sample   = glm::normalize(Sample);
            Sample *= math::RandomFloat();

            // Use an accelerating interpolation function so there are more samples close to the fragment
            float Scale = (float)i / (float)NumSSAOSamples;
            Scale       = math::Lerp(0.1, 1.0f, Scale * Scale);
            Sample *= Scale;

            // Send the sample to the shader
            FDString SampleUniformName = SPrintf(LUCID_TEXT("uSamples[%d]"), i);
            SSAOShader->SetVector(SampleUniformName, Sample);
            SampleUniformName.Free();
        }

        // Noise
        glm::vec2 Noise[16];
        for (i8 i = 0; i < 16; ++i)
        {
            Noise[i]   = math::RandomVec2();
            Noise[i].x = Noise[i].x * 2.0 - 1.0;
            Noise[i].y = Noise[i].y * 2.0 - 1.0;
        }

        SSAONoise = gpu::Create2DTexture(
          Noise, 4, 4, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::RG32F, gpu::ETexturePixelFormat::RG, 0, FSString{ "SSAONoise" });
        SSAONoise->Bind();
        SSAONoise->SetWrapSFilter(gpu::EWrapTextureFilter::REPEAT);
        SSAONoise->SetWrapTFilter(gpu::EWrapTextureFilter::REPEAT);
        SSAOShader->UseTexture(SSAO_NOISE, SSAONoise);
        SSAOShader->SetFloat(SSAO_RADIUS, SSAORadius);

        // Fame data buffers fences
        for (u8 i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
        {
            PersistentBuffersFences[i] = gpu::CreateFence("PersistentBufferFence");
        }

        {
            // Global data buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = GLOBAL_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            GlobalDataUBO = gpu::CreateImmutableBuffer(BufferDesc, COHERENT_WRITE_USAGE, "GlobalRenderUBO");
            GlobalDataUBO->Bind(gpu::EBufferBindPoint::WRITE);
            GlobalDataMappedPtr = (char*)GlobalDataUBO->MemoryMap(COHERENT_WRITE);
        }

        {
            // Prepass buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = PREPASS_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            PrepassDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, COHERENT_WRITE_USAGE, "PrepassDataSSBO");
            PrepassDataSSBO->Bind(gpu::EBufferBindPoint::WRITE);
            PrepassDataMappedPtr = (char*)PrepassDataSSBO->MemoryMap(COHERENT_WRITE);
        }

        {
            // Mesh resource data buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = ACTOR_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            ActorDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, COHERENT_WRITE_USAGE, "ActorDataSSBO");
            ActorDataSSBO->Bind(gpu::EBufferBindPoint::WRITE);
            ActorDataMappedPtr = (char*)ActorDataSSBO->MemoryMap(COHERENT_WRITE);
        }

        {
            // Instance data buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = INSTANCE_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            InstanceDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, COHERENT_WRITE_USAGE, "InstanceDataSSBO");
            InstanceDataSSBO->Bind(gpu::EBufferBindPoint::WRITE);
            InstanceDataMappedPtr = (char*)InstanceDataSSBO->MemoryMap(COHERENT_WRITE);
        }

#if DEVELOPMENT

        // Light bulbs
        LightsBillboardsPipelineState.ClearColorBufferColor    = FColor{ 0 };
        LightsBillboardsPipelineState.ClearDepthBufferValue    = 0;
        LightsBillboardsPipelineState.IsDepthTestEnabled       = true;
        LightsBillboardsPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        LightsBillboardsPipelineState.IsBlendingEnabled        = true;
        LightsBillboardsPipelineState.BlendFunctionSrc         = gpu::EBlendFunction::SRC_ALPHA;
        LightsBillboardsPipelineState.BlendFunctionDst         = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;
        LightsBillboardsPipelineState.BlendFunctionAlphaSrc    = gpu::EBlendFunction::ONE;
        LightsBillboardsPipelineState.BlendFunctionAlphaDst    = gpu::EBlendFunction::ONE;
        LightsBillboardsPipelineState.IsCullingEnabled         = false;
        LightsBillboardsPipelineState.IsSRGBFramebufferEnabled = false;
        LightsBillboardsPipelineState.IsDepthBufferReadOnly    = false;
        LightsBillboardsPipelineState.Viewport                 = LightpassPipelineState.Viewport;

        auto* LightBulbTextureResource = GEngine.GetTexturesHolder().Get(sole::rebuild("abd835d6-6aa9-4140-9442-9afe04a2b999"));
        LightBulbTextureResource->Acquire(false, true);
        LightBulbTexture = LightBulbTextureResource->TextureHandle;

        // Editor helpers
        EditorHelpersFramebuffer = gpu::CreateFramebuffer(FSString{ "HitMapMapFramebuffer" });
        EditorHelpersFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        HitMapTexture = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                  ResultResolution.y,
                                                  gpu::ETextureDataType::UNSIGNED_INT,
                                                  gpu::ETextureDataFormat::R32UI,
                                                  gpu::ETexturePixelFormat::RED_INTEGER,
                                                  0,
                                                  FSString{ "HitMapTexture" });

        HitMapTexture->Bind();
        HitMapTexture->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        HitMapTexture->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

        EditorHelpersFramebuffer->SetupColorAttachment(0, HitMapTexture);

        DistanceToCameraTexture = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                            ResultResolution.y,
                                                            gpu::ETextureDataType::FLOAT,
                                                            gpu::ETextureDataFormat::R32F,
                                                            gpu::ETexturePixelFormat::RED,
                                                            0,
                                                            FSString{ "DistanceToCameraTexture" });

        DistanceToCameraTexture->Bind();
        DistanceToCameraTexture->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        DistanceToCameraTexture->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

        EditorHelpersFramebuffer->SetupColorAttachment(1, DistanceToCameraTexture);

        EditorHelpersDepthStencilRenderbuffer =
          gpu::CreateRenderbuffer(gpu::ERenderbufferFormat::DEPTH24_STENCIL8, { ResultResolution.x, ResultResolution.y }, FSString{ "HitMapRenderbuffer" });

        EditorHelpersDepthStencilRenderbuffer->Bind();
        EditorHelpersFramebuffer->SetupDepthStencilAttachment(EditorHelpersDepthStencilRenderbuffer);

        EditorHelpersFramebuffer->IsComplete();

        CachedHitMap.Width  = ResultResolution.x;
        CachedHitMap.Height = ResultResolution.y;
        // @Note doesn't get freed, but it's probably okay as it should die with the editor
        CachedHitMap.CachedTextureData = (u32*)malloc(HitMapTexture->GetSizeInBytes());
        Zero(CachedHitMap.CachedTextureData, HitMapTexture->GetSizeInBytes());

        CachedDistanceToCameraMap.Width  = ResultResolution.x;
        CachedDistanceToCameraMap.Height = ResultResolution.y;
        // @Note doesn't get freed, but it's probably okay as it should die with the editor
        CachedDistanceToCameraMap.CachedTextureData = (float*)malloc(DistanceToCameraTexture->GetSizeInBytes()); //
        Zero(CachedDistanceToCameraMap.CachedTextureData, DistanceToCameraTexture->GetSizeInBytes());

        HitmapReadPBO       = gpu::CreatePixelBuffer("HitmapReadPixelBuffer_0", HitMapTexture->GetSizeInBytes());
        DistanceToCameraPBO = gpu::CreatePixelBuffer("DistanceToCameraReadPixelBuffer_0", HitMapTexture->GetSizeInBytes());

        // Timer
        FrameTimer = gpu::CreateTimer("FameTimer");

        // Debug lines
        DebugLinesPipelineState.IsDepthTestEnabled       = true;
        DebugLinesPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        DebugLinesPipelineState.IsBlendingEnabled        = true;
        DebugLinesPipelineState.BlendFunctionSrc         = gpu::EBlendFunction::ONE;
        DebugLinesPipelineState.BlendFunctionDst         = gpu::EBlendFunction::ONE;
        DebugLinesPipelineState.IsSRGBFramebufferEnabled = false;
        DebugLinesPipelineState.IsDepthBufferReadOnly    = false;
        DebugLinesPipelineState.LineWidth                = 2;
        DebugLinesPipelineState.Viewport                 = LightpassPipelineState.Viewport;
        // Create buffers and fences
        {
            for (int i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                gpu::FBufferDescription BufferDescription;
                BufferDescription.Data   = nullptr;
                BufferDescription.Offset = 0;
                BufferDescription.Size   = sizeof(glm::vec3) * 3 * MaxDebugLines;

                FDString BufferName        = SPrintf("DebugLinesVBO_%d", i);
                DebugLinesVertexBuffers[i] = gpu::CreateBuffer(BufferDescription, gpu::EBufferUsage::DYNAMIC_DRAW, BufferName);
            }
            FArray<gpu::FVertexAttribute> VertexAttributes{ 3, false };

            // Position and color
            constexpr u32 Stride = sizeof(glm::vec3) * 2 + sizeof(i32);
            VertexAttributes.Add({ 0, 3, EType::FLOAT, false, Stride, 0, 0, 0 });
            VertexAttributes.Add({ 1, 3, EType::FLOAT, false, Stride, sizeof(glm::vec3), 0, 0 });
            VertexAttributes.Add({ 2, 1, EType::INT_32, false, Stride, sizeof(glm::vec3) * 2, 0, 0 });

            DebugLinesVAO = gpu::CreateVertexArray("DebugLinesVAO", &VertexAttributes, nullptr, nullptr, gpu::EDrawMode::LINES, 0, 0, false);
            VertexAttributes.Free();
        }
#endif

        resources::CTextureResource* BlankTexture = GEngine.GetTexturesHolder().GetDefaultResource();
        BlankTexture->Acquire(false, true);
        BlankTextureBindlessHandle = BlankTexture->TextureHandle->GetBindlessHandle();
        BlankTexture->TextureHandle->MakeBindlessResident();
    }

    void CForwardRenderer::Cleanup()
    {
        assert(0); // @TODO Implement this properly!
        if (CurrentFrameVSNormalMap)
        {
            CurrentFrameVSNormalMap->Free();
            delete CurrentFrameVSNormalMap;
        }

        if (CurrentFrameVSPositionMap)
        {
            CurrentFrameVSPositionMap->Free();
            delete CurrentFrameVSPositionMap;
        }
    }

    void CForwardRenderer::Render(FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        for (u32 i = 0; i < InSceneToRender->StaticMeshes.GetLength(); ++i)
        {
            InSceneToRender->StaticMeshes.GetByIndex(i)->CalculateModelMatrix();
        }

        for (u32 i = 0; i < InSceneToRender->Terrains.GetLength(); ++i)
        {
            InSceneToRender->Terrains.GetByIndex(i)->CalculateModelMatrix();
        }

        SkyboxPipelineState.Viewport = LightpassPipelineState.Viewport = PrepassPipelineState.Viewport = InRenderView->Viewport;

        // Make sure we can use the persistent buffers
        const int    BufferIdx = GRenderStats.FrameNumber % FRAME_DATA_BUFFERS_COUNT;
        gpu::CFence* Fence     = PersistentBuffersFences[BufferIdx];

        while (!Fence->Wait(1))
        {
        }

        Fence->Free();
        delete Fence;

        // Check if any of the material buffer entries can be returned to the pool
        {
            std::vector<FFreeMaterialBufferEntries> EntriesStillInUse;
            for (FFreeMaterialBufferEntries& FreeEntries : FreeMaterialBuffersEntries)
            {
                if (FreeEntries.Fence->Wait(0))
                {
                    FMaterialDataBuffer& MaterialBuffer = MaterialDataBufferPerMaterialType[FreeEntries.MaterialType];
                    MaterialBuffer.FreeIndices.insert(MaterialBuffer.FreeIndices.end(), FreeEntries.Indices.begin(), FreeEntries.Indices.end());
                    FreeEntries.Fence->Free();
                    delete FreeEntries.Fence;
                }
                else
                {
                    EntriesStillInUse.push_back(FreeEntries);
                }
            }
            FreeMaterialBuffersEntries = EntriesStillInUse;
        }

#if DEVELOPMENT
        GRenderStats.NumDrawCalls = 0;
        ++GRenderStats.FrameNumber;
        FrameTimer->StartTimer();
#endif

        SetupGlobalRenderData(InRenderView);
        CreateMeshBatches(InSceneToRender);

        gpu::SetViewport(InRenderView->Viewport);

        gpu::PushDebugGroup("Shadow maps generation");
        GenerateShadowMaps(InSceneToRender);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("Prepass");
        Prepass(InSceneToRender, InRenderView);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("Lighting pass");
        LightingPass(InSceneToRender, InRenderView);
        gpu::PopDebugGroup();

#if DEVELOPMENT
        gpu::PushDebugGroup("Editor primitives");

        if (bDrawGrid)
        {
            gpu::PushDebugGroup("World grid");
            RenderWorldGrid(InRenderView);
            gpu::PopDebugGroup();
        }

        gpu::PushDebugGroup("Debug lines");
        RenderDebugLines(InRenderView);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("Billboards");
        DrawLightsBillboards(InSceneToRender, InRenderView);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("Hitmap");
        RenderEditorHelpers(InSceneToRender, InRenderView);
        gpu::PopDebugGroup();

        gpu::PopDebugGroup();
#endif

        gpu::PushDebugGroup("Gamma correction");
        DoGammaCorrection(LightingPassColorBuffers[GRenderStats.FrameNumber % NumFrameBuffers]);
        gpu::PopDebugGroup();
#if DEVELOPMENT
        GRenderStats.FrameTimeMiliseconds = FrameTimer->EndTimer();
        RemoveStaleDebugLines();
#endif

        PersistentBuffersFences[BufferIdx] = gpu::CreateFence("PersistentBufferFence");

        // Create fences for material data entries so we can recycle them once we're sure they're not used anymore
        for (auto& NewEntries : NewFreeMaterialBuffersEntries)
        {
            NewEntries.second.Fence = gpu::CreateFence("PersistentBufferFence");
            FreeMaterialBuffersEntries.push_back(NewEntries.second);
        }

        NewFreeMaterialBuffersEntries.clear();
    }

    void CForwardRenderer::ResetState()
    {

        for (const auto& It : MaterialDataBufferPerMaterialType)
        {
            It.second.GPUBuffer->MemoryUnmap();
            It.second.GPUBuffer->Free();

            delete It.second.GPUBuffer;
        }

        for (const auto& FreeEntries : FreeMaterialBuffersEntries)
        {
            FreeEntries.Fence->Free();
            delete FreeEntries.Fence;
        }

        for (const auto& It : NewFreeMaterialBuffersEntries)
        {
            It.second.Fence->Free();
            delete It.second.Fence;
        }

        MaterialDataBufferPerMaterialType.clear();
        FreeMaterialBuffersEntries.clear();
        NewFreeMaterialBuffersEntries.clear();
    }

    struct FBatchKey
    {
        gpu::CVertexArray* VertexArray  = nullptr;
        EMaterialType      MaterialType = EMaterialType::NONE;

        bool operator==(const FBatchKey& InRHS) const { return VertexArray == InRHS.VertexArray && MaterialType == InRHS.MaterialType; }
    };

    struct FBatchKeyHash
    {
        std::size_t operator()(const FBatchKey& Key) const { return (uintptr_t)(Key.VertexArray) ^ static_cast<u64>(Key.MaterialType); }
    };

    struct FMeshBatchBuilder
    {
        gpu::CShader*    BatchShader = nullptr;
        std::vector<u32> ActorEntryIndices;
        std::vector<u32> MaterialEntryIndices;

        std::vector<CMaterial*> BatchedMaterials; // this is only needed for the prepass and should be removed
    };

    FMaterialDataBuffer CForwardRenderer::CreateMaterialBuffer(CMaterial const* InMaterial, const u32& InMaterialBufferSize)
    {
        static gpu::FBufferDescription BufferDesc;
        BufferDesc.Size = INITIAL_MATERIAL_DATA_BUFFER_SIZE;

        FMaterialDataBuffer NewMaterialBuffer;
        NewMaterialBuffer.MaterialType = InMaterial->GetType();
        NewMaterialBuffer.NumEntries   = 1;
        NewMaterialBuffer.MaxEntries   = BufferDesc.Size / InMaterial->GetShaderDataSize();
        NewMaterialBuffer.GPUBuffer    = gpu::CreateImmutableBuffer(BufferDesc, COHERENT_WRITE_USAGE, "MaterialDataBuffer");
        NewMaterialBuffer.GPUBuffer->Bind(gpu::EBufferBindPoint::WRITE);
        NewMaterialBuffer.MappedPtr = (char*)NewMaterialBuffer.GPUBuffer->MemoryMap(COHERENT_WRITE);

        return NewMaterialBuffer;
    }

    void CForwardRenderer::FreeMaterialBufferEntry(const EMaterialType& InMaterialType, const i32& InIndex)
    {
        if (NewFreeMaterialBuffersEntries.find(InMaterialType) == NewFreeMaterialBuffersEntries.end())
        {
            FFreeMaterialBufferEntries FreeEntries;
            FreeEntries.MaterialType = InMaterialType;
            FreeEntries.Indices.push_back(InIndex);

            NewFreeMaterialBuffersEntries[InMaterialType] = FreeEntries;
        }
        else
        {
            NewFreeMaterialBuffersEntries[InMaterialType].Indices.push_back(InIndex);
        }
    }

    void CForwardRenderer::HandleMaterialBufferUpdateIfNecessary(CMaterial* Material)
    {
        // Find material data buffer
        if (MaterialDataBufferPerMaterialType.find(Material->GetType()) == MaterialDataBufferPerMaterialType.end())
        {
            // If there is no material buffer for this type of material - create one
            FMaterialDataBuffer NewMaterialBuffer                  = CreateMaterialBuffer(Material, INITIAL_MATERIAL_DATA_BUFFER_SIZE);
            MaterialDataBufferPerMaterialType[Material->GetType()] = NewMaterialBuffer;

            Material->MaterialBufferIndex = 0;
            Material->SetupShaderBuffer(NewMaterialBuffer.MappedPtr + (Material->MaterialBufferIndex * Material->GetShaderDataSize()));

            return;
        }

        auto& MaterialBuffer = MaterialDataBufferPerMaterialType[Material->GetType()];
        // If the material is dirty or was edited, return it's entry to the pool
        if (Material->TypeToFree != EMaterialType::NONE)
        {
            FreeMaterialBufferEntry(Material->TypeToFree, Material->MaterialBufferIndexToFree);
            Material->TypeToFree                = EMaterialType::NONE;
            Material->MaterialBufferIndexToFree = -1;
        }

        // Assign a material buffer index so this material can write it's data
        if (Material->MaterialBufferIndex == -1 || Material->IsMaterialDataDirty())
        {
            // Check if there are free indices - if yes, then get one
            if (MaterialBuffer.FreeIndices.size())
            {
                Material->MaterialBufferIndex = MaterialBuffer.FreeIndices.back();
                Material->SetupShaderBuffer(MaterialBuffer.MappedPtr + (Material->MaterialBufferIndex * Material->GetShaderDataSize()));

                MaterialBuffer.FreeIndices.pop_back();
            }
            // If there are free entries at the end - get once
            else if (MaterialBuffer.NumEntries < MaterialBuffer.MaxEntries)
            {
                Material->MaterialBufferIndex = MaterialBuffer.NumEntries++;
                Material->SetupShaderBuffer(MaterialBuffer.MappedPtr + (Material->MaterialBufferIndex * Material->GetShaderDataSize()));
            }
            // Otherwise just create a bigger buffer
            else
            {
                // Create a new buffer
                FMaterialDataBuffer NewMaterialBuffer = CreateMaterialBuffer(Material, MaterialBuffer.GPUBuffer->GetSize() * 2);
                Material->MaterialBufferIndex         = MaterialBuffer.NumEntries;
                NewMaterialBuffer.NumEntries          = ++NewMaterialBuffer.NumEntries;

                // Copy current data
                memcpy(MaterialBuffer.MappedPtr, NewMaterialBuffer.MappedPtr, MaterialBuffer.GPUBuffer->GetSize());

                // Remove the old buffer
                MaterialBuffer.GPUBuffer->MemoryUnmap();
                MaterialBuffer.GPUBuffer->Free();
                delete MaterialBuffer.GPUBuffer;

                // Update buffer mapping
                MaterialDataBufferPerMaterialType[Material->GetType()] = NewMaterialBuffer;
                MaterialBuffer                                         = NewMaterialBuffer;

                // Send updated data
                Material->SetupShaderBuffer(MaterialBuffer.MappedPtr + (Material->MaterialBufferIndex * Material->GetShaderDataSize()));

                // Recycle free indices if there are any
                auto& NewFreeEntriesIt = NewFreeMaterialBuffersEntries.find(Material->GetType());
                if (NewFreeEntriesIt != NewFreeMaterialBuffersEntries.end())
                {
                    NewMaterialBuffer.FreeIndices.insert(
                      NewMaterialBuffer.FreeIndices.end(), NewFreeEntriesIt->second.Indices.begin(), NewFreeEntriesIt->second.Indices.end());
                }

                std::vector<FFreeMaterialBufferEntries> FreeBufferEntries;
                for (const auto& FreeEntries : FreeMaterialBuffersEntries)
                {
                    if (FreeEntries.MaterialType == Material->GetType())
                    {
                        NewMaterialBuffer.FreeIndices.insert(
                          NewMaterialBuffer.FreeIndices.end(), NewFreeEntriesIt->second.Indices.begin(), NewFreeEntriesIt->second.Indices.end());
                    }
                    else
                    {
                        FreeBufferEntries.push_back(FreeEntries);
                    }
                }
                FreeMaterialBuffersEntries = FreeBufferEntries;
            }
        }
    }

    static inline u32 CalculateCurrentBufferOffset(const u32& InBufferSize)
    {
        const int BufferIdx = GRenderStats.FrameNumber % FRAME_DATA_BUFFERS_COUNT;
        return BufferIdx * InBufferSize;
    }

    void CForwardRenderer::CreateMeshBatches(FRenderScene* InSceneToRender)
    {
        std::unordered_map<FBatchKey, FMeshBatchBuilder, FBatchKeyHash> MeshBatchBuilders;
        std::unordered_map<EMaterialType, std::vector<FBatchKey>>       BatchKeyPerMaterialType;
        std::unordered_map<u32, u32>                                    ActorDataIdxByActorId;

        const auto BatchMesh = [&MeshBatchBuilders, &BatchKeyPerMaterialType](
                                 const FBatchKey& BatchKey, const u32& ActorEntryIndex, const u32& MaterialEntryIndex, CMaterial* Material) -> void {
            auto& BatchIt = MeshBatchBuilders.find(BatchKey);

            if (BatchIt == MeshBatchBuilders.end())
            {
                FMeshBatchBuilder BatchBuilder;
                BatchBuilder.ActorEntryIndices    = { ActorEntryIndex };
                BatchBuilder.MaterialEntryIndices = { MaterialEntryIndex };
                BatchBuilder.BatchShader          = Material->Shader;
                BatchBuilder.BatchedMaterials     = { Material };
                MeshBatchBuilders[BatchKey]       = BatchBuilder;

                if (BatchKeyPerMaterialType.find(BatchKey.MaterialType) == BatchKeyPerMaterialType.end())
                {
                    BatchKeyPerMaterialType[BatchKey.MaterialType] = { BatchKey };
                }
                else
                {
                    BatchKeyPerMaterialType[BatchKey.MaterialType].push_back(BatchKey);
                }
            }
            else
            {
                BatchIt->second.ActorEntryIndices.push_back(ActorEntryIndex);
                BatchIt->second.MaterialEntryIndices.push_back(MaterialEntryIndex);
                BatchIt->second.BatchedMaterials.push_back(Material);
            }
        };

        u32         ActorDataSize   = 0;
        const u32   ActorDataOffset = CalculateCurrentBufferOffset(ACTOR_DATA_BUFFER_SIZE);
        FActorData* ActorData       = (FActorData*)(ActorDataMappedPtr + ActorDataOffset);

        const auto FindActorEntryIndex =
          [&ActorDataIdxByActorId, &ActorData, &ActorDataSize](const u32& ActorId, const glm::mat4& ModelMatrix, const i8& NormalMultiplier) -> u32 {
            const auto ActorDataIdxIt = ActorDataIdxByActorId.find(ActorId);
            if (ActorDataIdxIt == ActorDataIdxByActorId.end())
            {
                u32 NewIndex                   = ActorDataIdxByActorId.size();
                ActorDataIdxByActorId[ActorId] = static_cast<u32>(NewIndex);

                ActorData->ModelMatrix      = ModelMatrix;
                ActorData->NormalMultiplier = NormalMultiplier;
                ActorData->ActorId          = ActorId;

                ActorData += 1;

                ActorDataSize += sizeof(FActorData);
                assert(ActorDataSize < ACTOR_DATA_BUFFER_SIZE);

                return NewIndex;
            }

            return ActorDataIdxIt->second;
        };

        // Create batch builders for static meshes
        for (u32 i = 0; i < InSceneToRender->StaticMeshes.GetLength(); ++i)
        {
            CStaticMesh* StaticMesh = InSceneToRender->StaticMeshes.GetByIndex(i);
            if (StaticMesh->MeshResource == nullptr)
            {
                LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a mesh resource", *StaticMesh->Name);
                continue;
            }

            if (!StaticMesh->bVisible)
            {
                continue;
            }

            const u32 ActorDataIdx = FindActorEntryIndex(StaticMesh->ActorId, StaticMesh->CachedModelMatrix, StaticMesh->bReverseNormals ? -1 : 1);

            // Send material updates to GPU
            for (u32 j = 0; j < StaticMesh->GetNumMaterialSlots(); ++j)
            {
                CMaterial* Material = StaticMesh->GetMaterialSlot(j);

                if (Material == nullptr)
                {
                    LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a material at slot %d", *StaticMesh->Name, j);
                    continue;
                }

                HandleMaterialBufferUpdateIfNecessary(Material);
            }

            // Iterate over submeshes and create batch builders
            for (u32 j = 0; j < StaticMesh->MeshResource->SubMeshes.GetLength(); ++j)
            {
                resources::FSubMesh* SubMesh         = StaticMesh->MeshResource->SubMeshes[j];
                CMaterial*           SubMeshMaterial = StaticMesh->GetMaterialSlot(SubMesh->MaterialIndex);

                const FBatchKey BatchKey{ SubMesh->VAO, SubMeshMaterial->GetType() };

                BatchMesh(BatchKey, ActorDataIdx, SubMeshMaterial->MaterialBufferIndex, SubMeshMaterial);
            }
        }

        // Create batch builders for terrain
        for (u32 i = 0; i < InSceneToRender->Terrains.GetLength(); ++i)
        {
            CTerrain* Terrain = InSceneToRender->Terrains.GetByIndex(i);

            if (!Terrain->bVisible)
            {
                continue;
            }

            const u32 ActorDataIdx = FindActorEntryIndex(Terrain->ActorId, Terrain->CachedModelMatrix, 1);

            // Send material updates to GPU
            CMaterial* TerrainMaterial = Terrain->GetTerrainMaterial();
            if (!TerrainMaterial)
            {
                LUCID_LOG(ELogLevel::ERR, "Terrain actor '%s' is missing a material", *Terrain->Name);
                continue;
            }

            HandleMaterialBufferUpdateIfNecessary(TerrainMaterial);

            // Create batch builder for this terrain
            const FBatchKey BatchKey{ Terrain->GetTerrainMesh()->SubMeshes[0]->VAO, Terrain->GetTerrainMaterial()->GetType() };
            BatchMesh(BatchKey, ActorDataIdx, Terrain->GetTerrainMaterial()->MaterialBufferIndex, Terrain->GetTerrainMaterial());
        }

        ActorDataSSBO->BindIndexed(1, gpu::EBufferBindPoint::SHADER_STORAGE, ActorDataSize, ActorDataOffset);

        // Create the batches themselves
        MeshBatches.clear();

        u32            InstanceDataSize   = 0;
        u32            TotalBatchedMeshes = 0;
        const u32      InstanceDataOffset = CalculateCurrentBufferOffset(INSTANCE_DATA_BUFFER_SIZE);
        FInstanceData* InstanceData       = (FInstanceData*)(InstanceDataMappedPtr + InstanceDataOffset);

        // This guarantees batches are sorted by material type
        for (const auto& It : BatchKeyPerMaterialType)
        {
            for (const auto& BatchKey : It.second)
            {
                const auto& BatchBuilder = MeshBatchBuilders[BatchKey];

                MeshBatches.push_back({});
                FMeshBatch& MeshBatch = MeshBatches[MeshBatches.size() - 1];

                MeshBatch.MeshVertexArray    = BatchKey.VertexArray;
                MeshBatch.Shader             = BatchBuilder.BatchShader;
                MeshBatch.BatchedSoFar       = TotalBatchedMeshes;
                MeshBatch.MaterialDataBuffer = &MaterialDataBufferPerMaterialType[BatchKey.MaterialType];
                MeshBatch.BatchSize          = BatchBuilder.ActorEntryIndices.size();
                MeshBatch.BatchedMaterials   = BatchBuilder.BatchedMaterials;

                // Build batch, write instance data to the gpu
                for (int i = 0; i < BatchBuilder.ActorEntryIndices.size(); ++i, ++TotalBatchedMeshes)
                {
                    //  instance data
                    InstanceData->ActorDataIdx    = BatchBuilder.ActorEntryIndices[i];
                    InstanceData->MaterialDataIdx = BatchBuilder.MaterialEntryIndices[i];

                    InstanceData += 1;
                    InstanceDataSize += sizeof(FInstanceData);

                    // @TODO handle this case
                    assert(InstanceDataSize < INSTANCE_DATA_BUFFER_SIZE);
                }
            }
        }

        InstanceDataSSBO->BindIndexed(2, gpu::EBufferBindPoint::SHADER_STORAGE, InstanceDataSize, InstanceDataOffset);

    } // namespace lucid::scene

    void CForwardRenderer::GenerateShadowMaps(FRenderScene* InSceneToRender)
    {
        // Prepare the pipeline state
        gpu::ConfigurePipelineState(ShadowMapGenerationPipelineState);

        ShadowMapFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        ShadowMapFramebuffer->DisableReadWriteBuffers();

        u8 PrevShadowMapQuality = 255;

        gpu::CShader* LastShadowMapShader = nullptr;
        for (int i = 0; i < InSceneToRender->AllLights.GetLength(); ++i)
        {
            CLight* Light = InSceneToRender->AllLights.GetByIndex(i);

            if (!Light->ShadowMap)
            {
                continue; // Light doesn't cast shadows
            }

            gpu::PushDebugGroup(*Light->Name);

            // @TODO This should happen only if light moves
            Light->UpdateLightSpaceMatrix(LightSettingsByQuality[Light->Quality]);

            gpu::CShader* CurrentShadowMapShader = Light->GetType() == ELightType::POINT ? ShadowCubeMapShader : ShadowMapShader;
            if (CurrentShadowMapShader != LastShadowMapShader)
            {
                CurrentShadowMapShader->Use();
                LastShadowMapShader = CurrentShadowMapShader;
            }

            Light->SetupShadowMapShader(CurrentShadowMapShader);

            // Check if we need to adjust the viewport based on light's shadow map quality
            if (Light->ShadowMap->GetQuality() != PrevShadowMapQuality)
            {
                PrevShadowMapQuality = Light->ShadowMap->GetQuality();
                gpu::SetViewport({ 0, 0, ShadowMapSizeByQuality[PrevShadowMapQuality].x, ShadowMapSizeByQuality[PrevShadowMapQuality].y });
            }

            // Setup light's shadow map texture
            ShadowMapFramebuffer->SetupDepthAttachment(Light->ShadowMap->GetShadowMapTexture());
            gpu::ClearBuffers(gpu::EGPUBuffer::DEPTH);

            // Static geometry
            for (const FMeshBatch& MeshBatch : MeshBatches)
            {
                MeshBatch.MeshVertexArray->Bind();
                CurrentShadowMapShader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
                MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchSize);
            }

            gpu::PopDebugGroup();
        }
    }

    void CForwardRenderer::Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        gpu::PushDebugGroup("Prepass");

        if (bEnableDepthPrepass)
        {
            gpu::PushDebugGroup("Depth prepass");

            u32                      PrepassDataSize     = 0;
            const u32&               PrepassBufferOffset = CalculateCurrentBufferOffset(PREPASS_DATA_BUFFER_SIZE);
            FForwardPrepassUniforms* PrepassDataPtr      = (FForwardPrepassUniforms*)(PrepassDataMappedPtr + PrepassBufferOffset);

            for (const auto& MeshBatch : MeshBatches)
            {
                for (const auto& BatchedMaterial : MeshBatch.BatchedMaterials)
                {
                    BatchedMaterial->SetupPrepassShader(PrepassDataPtr);

                    // Advance the pointer
                    PrepassDataPtr += 1;

                    // Add data size
                    PrepassDataSize += sizeof(FForwardPrepassUniforms);

                    assert(PrepassDataSize < PREPASS_DATA_BUFFER_SIZE);
                }
            }

            gpu::ConfigurePipelineState(PrepassPipelineState);

            PrepassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
            PrepassFramebuffer->SetupDrawBuffers();

            gpu::ClearBuffers(COLOR_AND_DEPTH);

            PrepassShader->Use();

            // Bind the SSBO
            PrepassDataSSBO->BindIndexed(3, gpu::EBufferBindPoint::SHADER_STORAGE, PREPASS_DATA_BUFFER_SIZE, PrepassBufferOffset);

            // Issue batches
            for (const auto& MeshBatch : MeshBatches)
            {
                PrepassShader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
                MeshBatch.MeshVertexArray->Bind();
                MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMaterials.size());
            }

            gpu::PopDebugGroup();
        }
        if (bEnableDepthPrepass && bEnableSSAO)
        {
            gpu::PushDebugGroup("SSAO");

            // Calculate SSAO
            const glm::vec2 NoiseTextureSize = { SSAONoise->GetWidth(), SSAONoise->GetHeight() };
            const glm::vec2 ViewportSize     = { InRenderView->Viewport.Width, InRenderView->Viewport.Height };
            const glm::vec2 NoiseScale       = ViewportSize / NoiseTextureSize;

            SSAOShader->Use();
            BindAndClearFramebuffer(SSAOFramebuffer);

            SSAOShader->UseTexture(SSAO_POSITIONS_VS, CurrentFrameVSPositionMap);
            SSAOShader->UseTexture(SSAO_NORMALS_VS, CurrentFrameVSNormalMap);
            SSAOShader->UseTexture(SSAO_NOISE, SSAONoise);
            SSAOShader->SetVector(SSAO_NOISE_SCALE, NoiseScale);
            SSAOShader->SetFloat(SSAO_BIAS, SSAOBias);
            SSAOShader->SetFloat(SSAO_RADIUS, SSAORadius);

            ScreenWideQuadVAO->Bind();
            ScreenWideQuadVAO->Draw();

            // Blur SSAO
            BlurFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
            BlurFramebuffer->SetupColorAttachment(0, SSAOBlurred);

            gpu::ClearBuffers(gpu::EGPUBuffer::COLOR);

            SimpleBlurShader->Use();
            SimpleBlurShader->UseTexture(SIMPLE_BLUR_TEXTURE, SSAOResult);
            SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_X, SimpleBlurXOffset);
            SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_Y, SimpleBlurYOffset);

            ScreenWideQuadVAO->Bind();
            ScreenWideQuadVAO->Draw();

            gpu::PopDebugGroup();
        }
        gpu::PopDebugGroup();
    }

    void CForwardRenderer::LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        // gpu::ConfigurePipelineState(InitialLightLightpassPipelineState);
        gpu::ConfigurePipelineState(LightpassPipelineState);

        gpu::CTexture* ColorBuffer = LightingPassColorBuffers[GRenderStats.FrameNumber % NumFrameBuffers];
        ColorBuffer->Bind();

        LightingPassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        LightingPassFramebuffer->SetupColorAttachment(0, ColorBuffer);
        LightingPassFramebuffer->SetupDrawBuffers();

        if (bEnableDepthPrepass)
        {
            gpu::ClearBuffers(gpu::EGPUBuffer::COLOR);
        }
        else
        {
            gpu::ClearBuffers(COLOR_AND_DEPTH);
        }

        RenderStaticMeshes(InSceneToRender, InRenderView);
        if (InSceneToRender->Skybox)
        {
            RenderSkybox(InSceneToRender->Skybox, InRenderView);
        }
    }

    void CForwardRenderer::RenderStaticMeshes(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        if (InScene->AllLights.GetLength() == 0)
        {
            RenderLightContribution(nullptr, InScene, InRenderView);
            return;
        }

        for (int i = 0; i < InScene->AllLights.GetLength(); ++i)
        {
            RenderLightContribution(InScene->AllLights.GetByIndex(i), InScene, InRenderView);
        }
    }

    void CForwardRenderer::RenderLightContribution(const CLight* InLight, const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        if (InLight)
        {
            gpu::PushDebugGroup(*InLight->Name);
        }

        for (const FMeshBatch& MeshBatch : MeshBatches)
        {
            gpu::PushDebugGroup(*MeshBatch.MeshVertexArray->GetName());
            MeshBatch.Shader->Use();
            if (InLight)
            {
                InLight->SetupShader(MeshBatch.Shader);
            }
            else
            {
                MeshBatch.Shader->SetInt(LIGHT_TYPE, NO_LIGHT);
            }

            MeshBatch.Shader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
            MeshBatch.MaterialDataBuffer->GPUBuffer->BindIndexed(3, gpu::EBufferBindPoint::SHADER_STORAGE);

            MeshBatch.MeshVertexArray->Bind();
            MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMaterials.size());
            gpu::PopDebugGroup();
        }
        if (InLight)
        {
            gpu::PopDebugGroup();
        }
    }

    void CForwardRenderer::BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer)
    {
        InFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers(COLOR_AND_DEPTH);
    }

    void CForwardRenderer::SetupGlobalRenderData(const FRenderView* InRenderView)
    {
        const u32& BufferOffset = CalculateCurrentBufferOffset(GLOBAL_DATA_BUFFER_SIZE);

        FGlobalRenderData* GlobalRenderData              = (FGlobalRenderData*)(GlobalDataMappedPtr + BufferOffset);
        GlobalRenderData->AmbientStrength                = AmbientStrength;
        GlobalRenderData->NumPCFsamples                  = NumSamplesPCF;
        GlobalRenderData->ProjectionMatrix               = InRenderView->Camera->GetProjectionMatrix();
        GlobalRenderData->ViewMatrix                     = InRenderView->Camera->GetViewMatrix();
        GlobalRenderData->ViewPos                        = InRenderView->Camera->Position;
        GlobalRenderData->ParallaxHeightScale            = 0.1f;
        GlobalRenderData->ViewportSize                   = glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height };
        GlobalRenderData->AmbientOcclusionBindlessHandle = bEnableSSAO ? SSAOBlurredBindlessHandle : BlankTextureBindlessHandle;
        GlobalRenderData->NearPlane                      = InRenderView->Camera->NearPlane;
        GlobalRenderData->FarPlane                       = InRenderView->Camera->FarPlane;

        GlobalDataUBO->BindIndexed(0, gpu::EBufferBindPoint::UNIFORM, GLOBAL_DATA_BUFFER_SIZE, BufferOffset);
    }

    inline void CForwardRenderer::RenderSkybox(const CSkybox* InSkybox, const FRenderView* InRenderView)
    {
        gpu::PushDebugGroup("Skybox");
        gpu::ConfigurePipelineState(SkyboxPipelineState);

        SkyboxShader->Use();

        SkyboxShader->UseTexture(SKYBOX_CUBEMAP, InSkybox->SkyboxCubemap);
        SkyboxShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        SkyboxShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());

        UnitCubeVAO->Bind();
        UnitCubeVAO->Draw();
        gpu::PopDebugGroup();
    }

    void CForwardRenderer::DrawLightsBillboards(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        if (!BillboardShader)
        {
            return;
        }

        gpu::ConfigurePipelineState(LightsBillboardsPipelineState);
        LightingPassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        // Keep it camera-oriented
        const glm::mat4 BillboardMatrix{ 1 };

        BillboardShader->Use();
        BillboardShader->SetMatrix(BILLBOARD_MATRIX, BillboardMatrix);
        BillboardShader->SetVector(BILLBOARD_VIEWPORT_SIZE, BillboardViewportSize);
        BillboardShader->UseTexture(BILLBOARD_TEXTURE, LightBulbTexture);
        BillboardShader->SetVector(VIEWPORT_SIZE, glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height });
        ScreenWideQuadVAO->Bind();

        for (int i = 0; i < InScene->AllLights.GetLength(); ++i)
        {
            CLight* Light = InScene->AllLights.GetByIndex(i);
            BillboardShader->SetVector(BILLBOARD_WORLD_POS, Light->Transform.Translation);
            BillboardShader->SetVector(BILLBOARD_COLOR_TINT, Light->Color);
            ScreenWideQuadVAO->Draw();
        }
    }

#if DEVELOPMENT
    void CForwardRenderer::RenderEditorHelpers(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        gpu::ConfigurePipelineState(EditorHelpersPipelineState);

        EditorHelpersFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        EditorHelpersFramebuffer->SetupDrawBuffers();

        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));

        EditorHelpersShader->Use();

        // Render static geometry
        for (const auto& MeshBatch : MeshBatches)
        {
            MeshBatch.MeshVertexArray->Bind();
            EditorHelpersShader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
            MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMaterials.size());
        }

        // Render lights quads
        ScreenWideQuadVAO->Bind();

        // Keep it camera-oriented
        const glm::mat4 BillboardMatrix{ 1 };
        EditorBillboardsShader->Use();
        EditorBillboardsShader->SetMatrix(BILLBOARD_MATRIX, BillboardMatrix);
        EditorBillboardsShader->SetVector(BILLBOARD_VIEWPORT_SIZE, BillboardViewportSize);
        EditorBillboardsShader->UseTexture(BILLBOARD_TEXTURE, LightBulbTexture);
        EditorBillboardsShader->SetVector(VIEWPORT_SIZE, glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height });

        for (int i = 0; i < InScene->AllLights.GetLength(); ++i)
        {
            CLight* Light = InScene->AllLights.GetByIndex(i);

            EditorBillboardsShader->SetVector(BILLBOARD_WORLD_POS, Light->Transform.Translation);
            EditorBillboardsShader->SetUInt(ACTOR_ID, Light->ActorId);

            ScreenWideQuadVAO->Draw();
        }

        // Get the result
        if (GRenderStats.FrameNumber == 1)
        {
            HitmapReadPBO->AsyncReadPixels(0, 0, 0, HitMapTexture->GetWidth(), HitMapTexture->GetHeight(), EditorHelpersFramebuffer);
            DistanceToCameraPBO->AsyncReadPixels(1, 0, 0, DistanceToCameraTexture->GetWidth(), DistanceToCameraTexture->GetHeight(), EditorHelpersFramebuffer);
        }
        else
        {
            // Check if the previous reads have finished and skip the read if not - it's not a problem to be 1-2 frames behind with this

            if (HitmapReadPBO->IsReady())
            {
                // Get the current result
                char* HitmapPixels = HitmapReadPBO->MapBuffer(gpu::EMapMode::READ_ONLY);
                memcpy(CachedHitMap.CachedTextureData, HitmapPixels, HitMapTexture->GetSizeInBytes());
                HitmapReadPBO->UnmapBuffer();

                // Async read the current frame
                HitmapReadPBO->AsyncReadPixels(0, 0, 0, HitMapTexture->GetWidth(), HitMapTexture->GetHeight(), EditorHelpersFramebuffer);
            }

            if (DistanceToCameraPBO->IsReady())
            {
                // Get the current result
                char* DistanceToCameraPixelsPixels = DistanceToCameraPBO->MapBuffer(gpu::EMapMode::READ_ONLY);
                memcpy(CachedDistanceToCameraMap.CachedTextureData, DistanceToCameraPixelsPixels, DistanceToCameraTexture->GetSizeInBytes());
                DistanceToCameraPBO->UnmapBuffer();

                // Async read the current frame
                DistanceToCameraPBO->AsyncReadPixels(
                  1, 0, 0, DistanceToCameraTexture->GetWidth(), DistanceToCameraTexture->GetHeight(), EditorHelpersFramebuffer);
            }
        }
    }

    // @TODO this should handle the batching
    void CForwardRenderer::RenderWithDefaultMaterial(const resources::CMeshResource* InMeshResource,
                                                     const u16&                      InSubMeshIndex,
                                                     const CLight*                   InLight,
                                                     const FRenderView*              InRenderView,
                                                     const glm::mat4&                InModelMatrix)
    {
        CMaterial* DefaultMaterial = GEngine.GetDefaultMaterial();
        DefaultMaterial->GetShader()->Use();

        if (InLight)
        {
            InLight->SetupShader(DefaultMaterial->GetShader());
        }
        else
        {
            DefaultMaterial->GetShader()->SetInt(LIGHT_TYPE, NO_LIGHT);
        }
        DefaultMaterial->GetShader()->SetMatrix(MODEL_MATRIX, InModelMatrix);
        // DefaultMaterial->SetupShader(DefaultMaterial->GetShader());

        if (InMeshResource)
        {
            InMeshResource->SubMeshes[InSubMeshIndex]->VAO->Bind();
            InMeshResource->SubMeshes[InSubMeshIndex]->VAO->Draw();
        }
        else
        {
            UnitCubeVAO->Bind();
            UnitCubeVAO->Draw();
        }
    }

    void CForwardRenderer::RenderWorldGrid(const FRenderView* InRenderView)
    {
        gpu::ConfigurePipelineState(WorldGridPipelineState);
        WorldGridShader->Use();

        ScreenWideQuadVAO->Bind();
        ScreenWideQuadVAO->Draw();
    }

    void CForwardRenderer::RenderDebugLines(const FRenderView* InRenderView)
    {
        // Make sure that the buffer is no longer used
        int BufferIdx = GRenderStats.FrameNumber % FRAME_DATA_BUFFERS_COUNT;

        gpu::CGPUBuffer* DebugLinesBuffer = DebugLinesVertexBuffers[BufferIdx];

        // Add lines visualizing the world space
        {
            const glm::mat3 ViewMatrixRotation = glm::mat3(InRenderView->Camera->GetViewMatrix());
            const glm::vec3 AxisCenter{ -0.95, -0.90, 0 };
            const glm::vec3 AxisEndX = AxisCenter + (ViewMatrixRotation * glm::vec3{ 0.05, 0, 0 });
            const glm::vec3 AxisEndY = AxisCenter + (ViewMatrixRotation * glm::vec3{ 0, 0.05, 0 });
            const glm::vec3 AxisEndZ = AxisCenter + (ViewMatrixRotation * glm::vec3{ 0, 0, 0.05 });

            AddDebugLine(AxisCenter, AxisEndX, { 1, 0, 0 }, { 1, 0, 0 }, 0, ESpaceType::CLIP_SPACE);
            AddDebugLine(AxisCenter, AxisEndY, { 0, 1, 0 }, { 0, 1, 0 }, 0, ESpaceType::CLIP_SPACE);
            AddDebugLine(AxisCenter, AxisEndZ, { 0, 0, 1 }, { 0, 0, 1 }, 0, ESpaceType::CLIP_SPACE);
        }

        // Copy lines data to GPU
        DebugLinesBuffer->Bind(gpu::EBufferBindPoint::VERTEX);
        char* DebugLinesMem =
          (char*)DebugLinesBuffer->MemoryMap((gpu::EBufferMapPolicy)(gpu::EBufferMapPolicy::BUFFER_WRITE | gpu::EBufferMapPolicy::BUFFER_UNSYNCHRONIZED));

        for (const FDebugLine& DebugLine : DebugLines)
        {
            memcpy(DebugLinesMem, &DebugLine.Start, sizeof(DebugLine.Start));
            DebugLinesMem += sizeof(DebugLine.Start);

            memcpy(DebugLinesMem, &DebugLine.StartColor, sizeof(DebugLine.StartColor));
            DebugLinesMem += sizeof(DebugLine.StartColor);

            memcpy(DebugLinesMem, &DebugLine.SpaceType, sizeof(DebugLine.SpaceType));
            DebugLinesMem += sizeof(DebugLine.SpaceType);

            memcpy(DebugLinesMem, &DebugLine.End, sizeof(DebugLine.End));
            DebugLinesMem += sizeof(DebugLine.End);

            memcpy(DebugLinesMem, &DebugLine.EndColor, sizeof(DebugLine.EndColor));
            DebugLinesMem += sizeof(DebugLine.EndColor);

            memcpy(DebugLinesMem, &DebugLine.SpaceType, sizeof(DebugLine.SpaceType));
            DebugLinesMem += sizeof(DebugLine.SpaceType);
        }

        DebugLinesBuffer->MemoryUnmap();
        DebugLinesBuffer->Unbind();

        // Prepare the pipeline
        DebugLinesShader->Use();
        DebugLinesVAO->Bind();

        ConfigurePipelineState(DebugLinesPipelineState);

        // Bind the updated buffer
        constexpr u32 Stride = sizeof(glm::vec3) * 2 + sizeof(i32);
        DebugLinesBuffer->BindAsVertexBuffer(0, Stride);
        DebugLinesBuffer->BindAsVertexBuffer(1, Stride);
        DebugLinesBuffer->BindAsVertexBuffer(2, Stride);

        // Draw the lines
        DebugLinesVAO->Draw(0, DebugLines.size() * 2);
    }

    void CForwardRenderer::UIDrawSettingsWindow()
    {
        ImGui::SetNextWindowSize({ 0, 0 });
        bool bOpen;
        ImGui::Begin("Renderer settings", &bOpen);
        {
            ImGui::DragFloat("Ambient strength", &AmbientStrength, 0.01, 0, 1);
            ImGui::DragInt("Num PCF samples", &NumSamplesPCF, 1, 0, 64);
            ImGui::Checkbox("Enable SSAO", &bEnableSSAO);
            ImGui::Checkbox("Draw grid", &bDrawGrid);
            if (ImGui::Checkbox("Depth prepass", &bEnableDepthPrepass))
            {
                if (bEnableDepthPrepass)
                {
                    LightpassPipelineState.DepthTestFunction     = gpu::EDepthTestFunction::EQUAL;
                    LightpassPipelineState.IsDepthBufferReadOnly = true;
                }
                else
                {
                    LightpassPipelineState.DepthTestFunction     = gpu::EDepthTestFunction::LEQUAL;
                    LightpassPipelineState.IsDepthBufferReadOnly = false;
                }
            }
        }
        ImGui::End();
    }

#endif

    void CForwardRenderer::DoGammaCorrection(gpu::CTexture* InTexture)
    {
        gpu::CTexture* FrameResultBuffer = FrameResultTextures[GRenderStats.FrameNumber % NumFrameBuffers];
        FrameResultBuffer->Bind();

        gpu::ConfigurePipelineState(GammaCorrectionPipelineState);
        FrameResultFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        FrameResultFramebuffer->SetupColorAttachment(0, FrameResultBuffer);
        FrameResultFramebuffer->SetupDrawBuffers();

        gpu::ClearBuffers(gpu::EGPUBuffer::COLOR);

        GammaCorrectionShader->Use();
        GammaCorrectionShader->SetFloat(GAMMA, Gamma);
        GammaCorrectionShader->UseTexture(SCENE_TEXTURE, InTexture);

        ScreenWideQuadVAO->Bind();
        ScreenWideQuadVAO->Draw();
    }

} // namespace lucid::scene