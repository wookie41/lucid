#include "scene/forward_renderer.hpp"

#include <set>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <resources/mesh_resource.hpp>
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

#include "scene/actors/lights.hpp"
#include "scene/camera.hpp"
#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"

#include "misc/basic_shapes.hpp"
#include "misc/math.hpp"

#include "resources/resources_holder.hpp"

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

    constexpr gpu::EBufferAccessPolicy UNSYNCHRONIZED_WRITE =
      (gpu::EBufferAccessPolicy)(gpu::EBufferAccessPolicy::BUFFER_WRITE | gpu::EBufferAccessPolicy::BUFFER_UNSYNCHRONIZED);

    constexpr gpu::EImmutableBufferUsage IMMUTABLE_DYNAMIC_WRITE_ONLY =
      (gpu::EImmutableBufferUsage)(gpu::EImmutableBufferUsage::IMM_BUFFER_WRITE | gpu::EImmutableBufferUsage::IMM_BUFFER_DYNAMIC);

    constexpr gpu::EGPUBuffer COLOR_AND_DEPTH = (gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH);

#if DEVELOPMENT
    static const FSString ACTOR_ID("uActorId");
#endif

    /* Buffers used when preparing data to be memcpy'ed to the GPU */

    static char STAGING_BUFFER_TINY_0[1024];
    static char STAGING_BUFFER_SMALL_0[1024 * 16];
    static char STAGING_BUFFER_SMALL_1[1024 * 16];
    static char STAGING_BUFFER_MEDIUM_0[1024 * 1024 * 5];

    constexpr u8 MAX_MESHES_PER_BATCH = 64;

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
        u32 ActorIdx;
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
        HitMapShader          = GEngine.GetShadersManager().GetShaderByName("Hitmap");
        BillboardHitMapShader = GEngine.GetShadersManager().GetShaderByName("BillboardHitmap");
        WorldGridShader       = GEngine.GetShadersManager().GetShaderByName("WorldGrid");
        DebugLinesShader      = GEngine.GetShadersManager().GetShaderByName("DebugLines");
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

        InitialLightLightpassPipelineState.ClearColorBufferColor = FColor{ 0 };
        InitialLightLightpassPipelineState.IsDepthTestEnabled    = true;
        InitialLightLightpassPipelineState.DepthTestFunction     = gpu::EDepthTestFunction::EQUAL;
        InitialLightLightpassPipelineState.IsBlendingEnabled     = true;
        InitialLightLightpassPipelineState.BlendFunctionSrc      = gpu::EBlendFunction::ONE;
        InitialLightLightpassPipelineState.BlendFunctionDst      = gpu::EBlendFunction::ZERO;
        InitialLightLightpassPipelineState.BlendFunctionAlphaSrc = gpu::EBlendFunction::ONE;
        InitialLightLightpassPipelineState.BlendFunctionAlphaDst = gpu::EBlendFunction::ZERO;
        // @TODO
        InitialLightLightpassPipelineState.IsCullingEnabled = false;
        // InitialLightLightpassPipelineState.IsCullingEnabled = true;
        // InitialLightLightpassPipelineState.CullMode = gpu::ECullMode::BACK;
        InitialLightLightpassPipelineState.IsSRGBFramebufferEnabled = false;
        InitialLightLightpassPipelineState.IsDepthBufferReadOnly    = true;

        LightpassPipelineState                       = InitialLightLightpassPipelineState;
        LightpassPipelineState.BlendFunctionDst      = gpu::EBlendFunction::ONE;
        LightpassPipelineState.BlendFunctionAlphaDst = gpu::EBlendFunction::ONE;

        SkyboxPipelineState                   = LightpassPipelineState;
        SkyboxPipelineState.IsBlendingEnabled = false;
        SkyboxPipelineState.DepthTestFunction = gpu::EDepthTestFunction::LEQUAL;

        GammaCorrectionPipelineState.ClearColorBufferColor    = FColor{ 0 };
        GammaCorrectionPipelineState.IsDepthTestEnabled       = false;
        GammaCorrectionPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        GammaCorrectionPipelineState.IsBlendingEnabled        = false;
        GammaCorrectionPipelineState.IsCullingEnabled         = false;
        GammaCorrectionPipelineState.IsSRGBFramebufferEnabled = false;
        GammaCorrectionPipelineState.IsDepthBufferReadOnly    = true;

#if DEVELOPMENT
        HitMapGenerationPipelineState                          = SkyboxPipelineState;
        HitMapGenerationPipelineState.IsDepthBufferReadOnly    = false;
        HitMapGenerationPipelineState.IsSRGBFramebufferEnabled = false;

        WorldGridPipelineState.IsDepthTestEnabled       = true;
        WorldGridPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        WorldGridPipelineState.IsBlendingEnabled        = true;
        WorldGridPipelineState.IsCullingEnabled         = false;
        WorldGridPipelineState.IsSRGBFramebufferEnabled = false;
        WorldGridPipelineState.IsDepthBufferReadOnly    = false;
        WorldGridPipelineState.BlendFunctionSrc         = gpu::EBlendFunction::SRC_ALPHA;
        WorldGridPipelineState.BlendFunctionDst         = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;
        WorldGridPipelineState.BlendFunctionAlphaSrc    = gpu::EBlendFunction::SRC_ALPHA;
        WorldGridPipelineState.BlendFunctionAlphaDst    = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;

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
        PrepassFramebuffer->SetupDepthAttachment(DepthStencilRenderBuffer);

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

        {
            // Global data buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = GLOBAL_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            GlobalDataUBO = gpu::CreateImmutableBuffer(BufferDesc, IMMUTABLE_DYNAMIC_WRITE_ONLY, "GlobalRenderUBO");

            for (u8 i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                GlobalDataBufferFences[i] = gpu::CreateFence("GlobalRenderDataFence");
            }
        }

        {
            // Prepass buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = PREPASS_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            PrepassDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, IMMUTABLE_DYNAMIC_WRITE_ONLY, "PrepassDataSSBO");

            for (u8 i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                PrepassDataBufferFences[i] = gpu::CreateFence("PrepassDataFence");
            }
        }

        {
            // Mesh resource data buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = ACTOR_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            ActorDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, IMMUTABLE_DYNAMIC_WRITE_ONLY, "ActorDataSSBO");

            for (u8 i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                ActorDataBufferFences[i] = gpu::CreateFence("ActorDataFence");
            }
        }

        {
            // Instance data buffers
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = INSTANCE_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            InstanceDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, IMMUTABLE_DYNAMIC_WRITE_ONLY, "InstanceDataSSBO");

            for (u8 i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                InstanceDataBufferFences[i] = gpu::CreateFence("InstanceDataFence");
            }
        }

        {
            // Material data buffer
            gpu::FBufferDescription BufferDesc;
            BufferDesc.Data   = nullptr;
            BufferDesc.Offset = 0;
            BufferDesc.Size   = MATERIAL_DATA_BUFFER_SIZE * FRAME_DATA_BUFFERS_COUNT;

            MaterialDataSSBO = gpu::CreateImmutableBuffer(BufferDesc, IMMUTABLE_DYNAMIC_WRITE_ONLY, "MaterialDataSSBO");

            for (u8 i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                MaterialDataBufferFences[i] = gpu::CreateFence("MaterialDataFence");
            }
        }

#if DEVELOPMENT

        // Light bulbs
        LightsBillboardsPipelineState.ClearColorBufferColor    = FColor{ 0 };
        LightsBillboardsPipelineState.ClearDepthBufferValue    = 0;
        LightsBillboardsPipelineState.IsDepthTestEnabled       = true;
        LightsBillboardsPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        LightsBillboardsPipelineState.IsBlendingEnabled        = true;
        LightsBillboardsPipelineState.BlendFunctionSrc         = gpu::EBlendFunction::SRC_ALPHA;
        LightsBillboardsPipelineState.BlendFunctionAlphaSrc    = gpu::EBlendFunction::SRC_ALPHA;
        LightsBillboardsPipelineState.BlendFunctionDst         = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;
        LightsBillboardsPipelineState.BlendFunctionAlphaDst    = gpu::EBlendFunction::ONE_MINUS_SRC_ALPHA;
        LightsBillboardsPipelineState.IsCullingEnabled         = false;
        LightsBillboardsPipelineState.IsSRGBFramebufferEnabled = false;
        LightsBillboardsPipelineState.IsDepthBufferReadOnly    = false;

        auto* LightBulbTextureResource = GEngine.GetTexturesHolder().Get(sole::rebuild("abd835d6-6aa9-4140-9442-9afe04a2b999"));
        LightBulbTextureResource->Acquire(false, true);
        LightBulbTexture = LightBulbTextureResource->TextureHandle;

        // HitMap generation
        HitMapTexture = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                  ResultResolution.y,
                                                  gpu::ETextureDataType::UNSIGNED_INT,
                                                  gpu::ETextureDataFormat::R32UI,
                                                  gpu::ETexturePixelFormat::RED_INTEGER,
                                                  0,
                                                  FSString{ "HitMapTexture" });

        HitMapDepthStencilRenderbuffer = gpu::CreateRenderbuffer(
          gpu::ERenderbufferFormat::DEPTH24_STENCIL8, { ResultResolution.x, ResultResolution.y }, FSString{ "HitMapRenderbuffer" });

        HitMapFramebuffer = gpu::CreateFramebuffer(FSString{ "HitMapMapFramebuffer" });
        HitMapFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        HitMapTexture->Bind();
        HitMapTexture->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        HitMapTexture->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

        HitMapFramebuffer->SetupColorAttachment(0, HitMapTexture);

        HitMapDepthStencilRenderbuffer->Bind();
        HitMapFramebuffer->SetupDepthStencilAttachment(HitMapDepthStencilRenderbuffer);

        HitMapFramebuffer->IsComplete();

        CachedHitMap.Width  = ResultResolution.x;
        CachedHitMap.Height = ResultResolution.y;
        CachedHitMap.CachedTextureData =
          (u32*)malloc(HitMapTexture->GetSizeInBytes()); // @Note doesn't get freed, but it's probably okay as it should die with the editor
        Zero(CachedHitMap.CachedTextureData, HitMapTexture->GetSizeInBytes());

        // Timer
        FrameTimer = gpu::CreateTimer("FameTimer");

        // Debug lines
        DebugLinesPipelineState.IsDepthTestEnabled       = true;
        DebugLinesPipelineState.DepthTestFunction        = gpu::EDepthTestFunction::LEQUAL;
        DebugLinesPipelineState.IsBlendingEnabled        = false;
        DebugLinesPipelineState.IsSRGBFramebufferEnabled = true;
        DebugLinesPipelineState.IsDepthBufferReadOnly    = false;
        DebugLinesPipelineState.LineWidth                = 2;

        // Create buffers and fences
        {
            for (int i = 0; i < FRAME_DATA_BUFFERS_COUNT; ++i)
            {
                gpu::FBufferDescription BufferDescription;
                BufferDescription.Data   = nullptr;
                BufferDescription.Offset = 0;
                BufferDescription.Size   = sizeof(glm::vec3) * 3 * MaxDebugLines;

                FDString BufferName        = SPrintf("DebugLinesVBO_%d", i);
                DebugLinesVertexBuffers[i] = gpu::CreateBuffer(BufferDescription, gpu::EBufferUsage::DYNAMIC, BufferName);

                DebugLinesFences[i] = gpu::CreateFence("DebugLinesFence");
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
    }

    void CForwardRenderer::Cleanup()
    {
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

        SkyboxPipelineState.Viewport = LightpassPipelineState.Viewport = PrepassPipelineState.Viewport = InRenderView->Viewport;

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

        gpu::PushDebugGroup("Billboards");
        DrawLightsBillboards(InSceneToRender, InRenderView);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("World grid");
        RenderWorldGrid(InRenderView);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("Debug lines");
        RenderDebugLines(InRenderView);
        gpu::PopDebugGroup();

        gpu::PushDebugGroup("Hitmap");
        GenerateHitmap(InSceneToRender, InRenderView);
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
    }

    struct FBatchKey
    {
        gpu::CVertexArray* VertexArray = nullptr;
        scene::CMaterial*  Material    = nullptr;

        bool operator==(const FBatchKey& InRHS) const { return VertexArray == InRHS.VertexArray && Material->GetType() == InRHS.Material->GetType(); }
    };

    struct FBatchKeyHash
    {
        std::size_t operator()(const FBatchKey& Key) const { return (uintptr_t)(Key.VertexArray) ^ (uintptr_t)(Key.Material->GetType()); }
    };

    struct FMeshBatchBuilder
    {
        std::vector<CStaticMesh*> StaticMeshes; // actor that owns the given mesh
        std::vector<CMaterial*>   Materials; // Instances of the same material
        u32                       MeshDataResourceIdx;
    };

    void CForwardRenderer::CreateMeshBatches(FRenderScene* InSceneToRender)
    {
        std::unordered_map<FBatchKey, FMeshBatchBuilder, FBatchKeyHash> MeshBatchBuilders;
        std::unordered_map<u32, u32>                                    ActorDataIdxByActorId;

        FActorData* ActorData = (FActorData*)STAGING_BUFFER_SMALL_0;

        // Create batch builders
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

            const auto ActorDataIdxIt = ActorDataIdxByActorId.find(StaticMesh->ActorId);
            if (ActorDataIdxIt == ActorDataIdxByActorId.end())
            {
                ActorDataIdxByActorId[StaticMesh->ActorId] = static_cast<u32>(ActorDataIdxByActorId.size());

                ActorData->ModelMatrix      = StaticMesh->CachedModelMatrix;
                ActorData->NormalMultiplier = StaticMesh->bReverseNormals ? -1 : 1;
                ActorData->ActorId          = StaticMesh->ActorId;

                ActorData += 1;
            }

            for (u32 j = 0; j < StaticMesh->MeshResource->SubMeshes.GetLength(); ++j)
            {
                resources::FSubMesh* SubMesh         = StaticMesh->MeshResource->SubMeshes[j];
                CMaterial*           SubMeshMaterial = StaticMesh->GetMaterialSlot(SubMesh->MaterialIndex);

                if (SubMeshMaterial == nullptr)
                {
                    LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a material for submesh %d", *StaticMesh->Name, j);
                    continue;
                }

                const FBatchKey BatchKey{ SubMesh->VAO, SubMeshMaterial };
                auto&           BatchIt = MeshBatchBuilders.find(BatchKey);

                if (BatchIt == MeshBatchBuilders.end())
                {
                    FMeshBatchBuilder BatchBuilder;
                    BatchBuilder.StaticMeshes.push_back(StaticMesh);
                    BatchBuilder.Materials.push_back(SubMeshMaterial);
                    MeshBatchBuilders[BatchKey] = BatchBuilder;
                }
                else
                {
                    BatchIt->second.StaticMeshes.push_back(StaticMesh);
                    BatchIt->second.Materials.push_back(SubMeshMaterial);
                }
            }
        }

        // Send actor data
        static u32 ActorDataSize;
        static u32 ActorBufferOffset;
        ActorDataSize = ActorDataIdxByActorId.size() * sizeof(FActorData);
        ActorBufferOffset =
          SendDataToGPU(STAGING_BUFFER_SMALL_0, ActorDataSize, ActorDataSSBO, ActorDataBufferFences, ACTOR_DATA_BUFFER_SIZE, "ActorDataFence");

        ActorDataSSBO->BindIndexed(1, gpu::EBufferBindPoint::SHADER_STORAGE, ActorDataSize, ActorBufferOffset);

        // Create the batches themselves
        MeshBatches.clear();

        u32            InstanceDataSize   = 0;
        u32            TotalBatchedMeshes = 0;
        FInstanceData* InstanceData       = (FInstanceData*)STAGING_BUFFER_SMALL_0;

        u32   MaterialDataSize = 0;
        char* MaterialDataPtr  = STAGING_BUFFER_MEDIUM_0;

        for (auto& BatchBuilder : MeshBatchBuilders)
        {
            MeshBatches.push_back({});
            FMeshBatch& MeshBatch = MeshBatches[MeshBatches.size() - 1];

            MeshBatch.MeshVertexArray    = BatchBuilder.first.VertexArray;
            MeshBatch.Shader             = BatchBuilder.first.Material->Shader;
            MeshBatch.BatchedSoFar       = TotalBatchedMeshes;
            MeshBatch.MaterialDataOffset = MaterialDataSize;
            MeshBatch.MaterialDataSize   = 0;

            // Build batches, prepare data to be sent to the GPU
            for (int i = 0; i < BatchBuilder.second.Materials.size(); ++i, ++TotalBatchedMeshes)
            {
                FBatchedMesh BatchedMesh;
                BatchedMesh.Material         = BatchBuilder.second.Materials[i];
                BatchedMesh.NormalMultiplier = BatchBuilder.second.StaticMeshes[i]->GetReverseNormals() ? -1 : 1;
                MeshBatch.BatchedMeshes.push_back(BatchedMesh);

                //  instance data
                InstanceData->ActorIdx = ActorDataIdxByActorId[BatchBuilder.second.StaticMeshes[i]->ActorId];

                InstanceData += 1;
                InstanceDataSize += sizeof(FInstanceData);

                // Material data
                const u32 BytesWritten = BatchedMesh.Material->SetupShader(MaterialDataPtr);
                MaterialDataSize += BytesWritten;
                MeshBatch.MaterialDataSize += BytesWritten;
                MaterialDataPtr += BytesWritten;

                // Limit batch size to 64
                if (MeshBatch.BatchedMeshes.size() == MAX_MESHES_PER_BATCH)
                {
                    MeshBatches.push_back({});
                    FMeshBatch NextBatch         = MeshBatches[MeshBatches.size() - 1];
                    NextBatch.MeshVertexArray    = MeshBatch.MeshVertexArray;
                    NextBatch.Shader             = BatchBuilder.first.Material->GetShader();
                    NextBatch.BatchedSoFar       = TotalBatchedMeshes;
                    NextBatch.MaterialDataOffset = MaterialDataSize;
                    NextBatch.MaterialDataSize   = 0;
                    MeshBatch                    = NextBatch;
                }
            }
        }

        // @TODO handle this case
        assert(InstanceDataSize < INSTANCE_DATA_BUFFER_SIZE);
        assert(MaterialDataSize < MATERIAL_DATA_BUFFER_SIZE);

        // Send common instance data to the GPU
        {
            const u32 BufferOffset = SendDataToGPU(
              STAGING_BUFFER_SMALL_0, InstanceDataSize, InstanceDataSSBO, InstanceDataBufferFences, INSTANCE_DATA_BUFFER_SIZE, "InstanceDataFence");
            InstanceDataSSBO->BindIndexed(2, gpu::EBufferBindPoint::SHADER_STORAGE, InstanceDataSize, BufferOffset);
        }

        // Send material data to the GPU
        {
            static u32 BufferOffset;

            BufferOffset = SendDataToGPU(
              STAGING_BUFFER_MEDIUM_0, MaterialDataSize, MaterialDataSSBO, MaterialDataBufferFences, MATERIAL_DATA_BUFFER_SIZE, "MaterialDataFence");
            MaterialDataSSBO->BindIndexed(3, gpu::EBufferBindPoint::SHADER_STORAGE, MaterialDataSize, BufferOffset);
        }
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
                gpu::SetViewport({ 0, 0, (u32)ShadowMapSizeByQuality[PrevShadowMapQuality].x, (u32)ShadowMapSizeByQuality[PrevShadowMapQuality].y });
            }

            // Setup light's shadow map texture
            ShadowMapFramebuffer->SetupDepthAttachment(Light->ShadowMap->GetShadowMapTexture());
            gpu::ClearBuffers(gpu::EGPUBuffer::DEPTH);

            // Static geometry
            for (const FMeshBatch& MeshBatch : MeshBatches)
            {
                MeshBatch.MeshVertexArray->Bind();
                CurrentShadowMapShader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
                MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMeshes.size());
            }

            gpu::PopDebugGroup();
        }
    }

    void CForwardRenderer::Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        gpu::PushDebugGroup("Z pre pass");

        const int BufferIdx = GRenderStats.FrameNumber % FRAME_DATA_BUFFERS_COUNT;

        const u32 DataBufferOffset = BufferIdx * PREPASS_DATA_BUFFER_SIZE;

        u32                      PrepassDataSize = 0;
        FForwardPrepassUniforms* PrepassDataPtr  = (FForwardPrepassUniforms*)STAGING_BUFFER_SMALL_0;

        for (const auto& MeshBatch : MeshBatches)
        {
            for (const auto& BatchedMesh : MeshBatch.BatchedMeshes)
            {
                BatchedMesh.Material->SetupPrepassShader(PrepassDataPtr);

                // Advance the pointer
                PrepassDataPtr += 1;

                // Add data size
                PrepassDataSize += sizeof(FForwardPrepassUniforms);
            }
        }

        static int prepasscount = 0;

        if (prepasscount < 3)
        // Send data to the GPU
        {
            prepasscount += 1;
            // Make sure the buffer is not used and create a new fence
            gpu::CFence* Fence = PrepassDataBufferFences[BufferIdx];

            Fence->Wait(0);
            Fence->Free();

            delete Fence;
            PrepassDataBufferFences[BufferIdx] = gpu::CreateFence("PrepassDataFence");

            // memcpy the data
            PrepassDataSSBO->Bind(gpu::EBufferBindPoint::SHADER_STORAGE);
            void* VideoMemPtr = PrepassDataSSBO->MemoryMap(UNSYNCHRONIZED_WRITE, PREPASS_DATA_BUFFER_SIZE, DataBufferOffset);
            memcpy(VideoMemPtr, STAGING_BUFFER_SMALL_0, PrepassDataSize);
            PrepassDataSSBO->MemoryUnmap();
        }

        // Prepare pipeline
        gpu::ConfigurePipelineState(PrepassPipelineState);

        PrepassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        PrepassFramebuffer->SetupDrawBuffers();

        gpu::ClearBuffers(COLOR_AND_DEPTH);

        PrepassShader->Use();

        // Bind the SSBO
        PrepassDataSSBO->BindIndexed(3, gpu::EBufferBindPoint::SHADER_STORAGE, DataBufferOffset, DataBufferOffset);

        // Issue batches
        for (const auto& MeshBatch : MeshBatches)
        {
            PrepassShader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
            MeshBatch.MeshVertexArray->Bind();
            MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMeshes.size());
        }

        gpu::PopDebugGroup();

        // Calculate SSAO

        const glm::vec2 NoiseTextureSize = { SSAONoise->GetWidth(), SSAONoise->GetHeight() };
        const glm::vec2 ViewportSize     = { InRenderView->Viewport.Width, InRenderView->Viewport.Height };
        const glm::vec2 NoiseScale       = ViewportSize / NoiseTextureSize;

        gpu::PushDebugGroup("SSAO");

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

        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR));

        SimpleBlurShader->Use();
        SimpleBlurShader->UseTexture(SIMPLE_BLUR_TEXTURE, SSAOResult);
        SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_X, SimpleBlurXOffset);
        SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_Y, SimpleBlurYOffset);

        ScreenWideQuadVAO->Bind();
        ScreenWideQuadVAO->Draw();

        gpu::PopDebugGroup();
    }

    void CForwardRenderer::LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        gpu::ConfigurePipelineState(InitialLightLightpassPipelineState);

        gpu::CTexture* ColorBuffer = LightingPassColorBuffers[GRenderStats.FrameNumber % NumFrameBuffers];
        ColorBuffer->Bind();

        LightingPassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        LightingPassFramebuffer->SetupColorAttachment(0, ColorBuffer);
        LightingPassFramebuffer->SetupDrawBuffers();

        gpu::ClearBuffers(gpu::EGPUBuffer::COLOR);

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

        RenderLightContribution(InScene->AllLights.GetByIndex(0), InScene, InRenderView);

        // Switch blending so we render the rest of the lights additively
        gpu::ConfigurePipelineState(LightpassPipelineState);
        for (int i = 1; i < InScene->AllLights.GetLength(); ++i)
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
            MaterialDataSSBO->BindIndexed(3, gpu::EBufferBindPoint::SHADER_STORAGE, MeshBatch.MaterialDataSize, MeshBatch.MaterialDataOffset);

            MeshBatch.MeshVertexArray->Bind();
            MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMeshes.size());
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
        FGlobalRenderData* GlobalRenderData              = (FGlobalRenderData*)STAGING_BUFFER_TINY_0;
        GlobalRenderData->AmbientStrength                = AmbientStrength;
        GlobalRenderData->NumPCFsamples                  = NumSamplesPCF;
        GlobalRenderData->ProjectionMatrix               = InRenderView->Camera->GetProjectionMatrix();
        GlobalRenderData->ViewMatrix                     = InRenderView->Camera->GetViewMatrix();
        GlobalRenderData->ViewPos                        = InRenderView->Camera->Position;
        GlobalRenderData->ParallaxHeightScale            = 0.1f;
        GlobalRenderData->ViewportSize                   = glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height };
        GlobalRenderData->AmbientOcclusionBindlessHandle = SSAOBlurredBindlessHandle;
        GlobalRenderData->NearPlane                      = InRenderView->Camera->NearPlane;
        GlobalRenderData->FarPlane                       = InRenderView->Camera->FarPlane;
        GlobalRenderData->ParallaxHeightScale            = InRenderView->Camera->FarPlane;

        const u32& BufferOffset = SendDataToGPU(
          STAGING_BUFFER_TINY_0, sizeof(FGlobalRenderData), GlobalDataUBO, GlobalDataBufferFences, GLOBAL_DATA_BUFFER_SIZE, "GlobalDataFence");
        GlobalDataUBO->BindIndexed(0, gpu::EBufferBindPoint::UNIFORM, sizeof(FGlobalRenderData), BufferOffset);
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
        const glm::mat4 BillboardMatrix = {
            { InRenderView->Camera->RightVector, 0 }, { InRenderView->Camera->UpVector, 0 }, { -InRenderView->Camera->FrontVector, 0 }, { 0, 0, 0, 1 }
        };

        BillboardShader->Use();
        BillboardShader->SetMatrix(BILLBOARD_MATRIX, BillboardMatrix);
        BillboardShader->SetVector(BILLBOARD_VIEWPORT_SIZE, BillboardViewportSize);
        BillboardShader->UseTexture(BILLBOARD_TEXTURE, LightBulbTexture);
        BillboardShader->SetVector(VIEWPORT_SIZE, glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height });
        BillboardShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        BillboardShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());
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
    void CForwardRenderer::GenerateHitmap(const FRenderScene* InScene, const FRenderView* InRenderView) const
    {
        // We do it in a separate pass just for simplicity
        // It might not be the most efficient way to do it, but that way we avoid the need to maintain two
        // versions of the lighting pass shader - one used in tools that also writes the ids, and one for cooked games
        gpu::ConfigurePipelineState(HitMapGenerationPipelineState);

        HitMapFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        HitMapFramebuffer->SetupDrawBuffers();

        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));

        HitMapShader->Use();

        // Render static geometry
        for (const auto& MeshBatch : MeshBatches)
        {
            MeshBatch.MeshVertexArray->Bind();
            HitMapShader->SetInt(MESH_BATCH_OFFSET, MeshBatch.BatchedSoFar);
            MeshBatch.MeshVertexArray->DrawInstanced(MeshBatch.BatchedMeshes.size());
        }

        // Render lights quads
        ScreenWideQuadVAO->Bind();

        // Keep it camera-oriented
        const glm::mat4 BillboardMatrix = {
            { InRenderView->Camera->RightVector, 0 }, { InRenderView->Camera->UpVector, 0 }, { -InRenderView->Camera->FrontVector, 0 }, { 0, 0, 0, 1 }
        };

        BillboardHitMapShader->Use();
        BillboardHitMapShader->SetMatrix(BILLBOARD_MATRIX, BillboardMatrix);
        BillboardHitMapShader->SetVector(BILLBOARD_VIEWPORT_SIZE, BillboardViewportSize);
        BillboardHitMapShader->UseTexture(BILLBOARD_TEXTURE, LightBulbTexture);
        BillboardHitMapShader->SetVector(VIEWPORT_SIZE, glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height });
        BillboardHitMapShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        BillboardHitMapShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());

        for (int i = 0; i < InScene->AllLights.GetLength(); ++i)
        {
            CLight* Light = InScene->AllLights.GetByIndex(i);

            BillboardHitMapShader->SetVector(BILLBOARD_WORLD_POS, Light->Transform.Translation);
            BillboardHitMapShader->SetUInt(ACTOR_ID, Light->ActorId);

            ScreenWideQuadVAO->Draw();
        }

        // Get the result
        HitMapFramebuffer->ReadPixels(0, 0, HitMapTexture->GetWidth(), HitMapTexture->GetHeight(), CachedHitMap.CachedTextureData);
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

        gpu::CGPUBuffer* DebugLinesBuffer      = DebugLinesVertexBuffers[BufferIdx];
        gpu::CFence*     DebugLinesBufferFence = DebugLinesFences[BufferIdx];

        DebugLinesBufferFence->Wait(0);

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
        char* DebugLinesMem = (char*)DebugLinesBuffer->MemoryMap(
          (gpu::EBufferAccessPolicy)(gpu::EBufferAccessPolicy::BUFFER_WRITE | gpu::EBufferAccessPolicy::BUFFER_UNSYNCHRONIZED));

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

        // Remove the old fence and create a new one
        DebugLinesBufferFence->Free();
        delete DebugLinesBufferFence;
        DebugLinesFences[BufferIdx] = gpu::CreateFence("DebugLinesFence");
    }

    void CForwardRenderer::UIDrawSettingsWindow()
    {
        ImGui::SetNextWindowSize({ 0, 0 });
        ImGui::Begin("Renderer settings");
        {
            ImGui::DragFloat("Ambient strength", &AmbientStrength, 0.01, 0, 1);
            ImGui::DragInt("Num PCF samples", &NumSamplesPCF, 1, 0, 64);
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

    u32 CForwardRenderer::SendDataToGPU(void const*      InData,
                                        const u32&       InDataSize,
                                        gpu::CGPUBuffer* InBuffer,
                                        gpu::CFence*     InFences[],
                                        const u32&       InBufferSize,
                                        const FString&   InFenceName)
    {
        // Send common instance data to the GPU
        // Make sure the buffer is not used and create a new fence
        const int    BufferIdx    = GRenderStats.FrameNumber % FRAME_DATA_BUFFERS_COUNT;
        const u32    BufferOffset = BufferIdx * InBufferSize;
        gpu::CFence* Fence        = InFences[BufferIdx];

        Fence->Wait(0);
        Fence->Free();

        delete Fence;
        InFences[BufferIdx] = gpu::CreateFence(InFenceName);

        gpu::FBufferDescription Desc;
        Desc.Data   = (void*)InData;
        Desc.Size   = InDataSize;
        Desc.Offset = BufferOffset;

        // memcpy the data
        InBuffer->Bind(gpu::EBufferBindPoint::WRITE);
        void* VideoMemPtr = InBuffer->MemoryMap(UNSYNCHRONIZED_WRITE, InBufferSize, BufferOffset);
        memcpy(VideoMemPtr, InData, InDataSize);
        InBuffer->MemoryUnmap();

        return BufferOffset;
    }

} // namespace lucid::scene