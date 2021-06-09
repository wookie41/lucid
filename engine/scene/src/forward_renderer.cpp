#include "scene/forward_renderer.hpp"

#include <glm/gtx/matrix_decompose.hpp>
#include <resources/mesh_resource.hpp>
#include "GL/glew.h"
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
    static const glm::mat4 IDENTITY_MATRIX{ 1 };
    static const glm::vec3 WORLD_UP{ 0, 1, 0 };

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

    // Shader-wide uniforms
    static const FSString AMBIENT_STRENGTH("uAmbientStrength");
    static const FSString NUM_OF_PCF_SAMPLES("uNumSamplesPCF");
    static const FSString AMBIENT_OCCLUSION("uAmbientOcclusion");
    static const FSString NEAR_PLANE("uNearPlane");
    static const FSString FAR_PLANE("uFarPlane");

    static const FSString VIEW_POSITION("uViewPos");

    static const FSString MODEL_MATRIX("uModel");
    static const FSString REVERSE_NORMALS("uReverseNormals");
    static const FSString VIEW_MATRIX("uView");
    static const FSString PROJECTION_MATRIX("uProjection");

    static const FSString SKYBOX_CUBEMAP("uSkybox");

    static const FSString PARALLAX_HEIGHT_SCALE("uParallaxHeightScale");

    static const FSString FLAT_COLOR("uFlatColor");

    static const FSString GAMMA("uGamma");
    static const FSString SCENE_TEXTURE("uSceneTexture");

#if DEVELOPMENT
    static const FSString ACTOR_ID("uActorId");
#endif

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

        // Create a common depth-stencil attachment for both framebuffers
        DepthStencilRenderBuffer =
          gpu::CreateRenderbuffer(gpu::ERenderbufferFormat::DEPTH24_STENCIL8, ResultResolution, FSString{ "LightingPassRenderbuffer" });

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

        if (!PrepassFramebuffer->IsComplete())
        {
            LUCID_LOG(ELogLevel::ERR, LUCID_TEXT("Failed to setup the prepass framebuffer"));
            return;
        }

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

        // Create color attachment for the lighting pass framebuffer
        LightingPassColorBuffer = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                            ResultResolution.y,
                                                            gpu::ETextureDataType::FLOAT,
                                                            gpu::ETextureDataFormat::RGBA,
                                                            gpu::ETexturePixelFormat::RGBA,
                                                            0,
                                                            FSString{ "LightingPassColorBuffer" });

        // Setup the lighting pass framebuffer
        LightingPassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        LightingPassColorBuffer->Bind();
        LightingPassColorBuffer->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        LightingPassColorBuffer->SetMagFilter(gpu::EMagTextureFilter::NEAREST);
        LightingPassFramebuffer->SetupColorAttachment(0, LightingPassColorBuffer);

        DepthStencilRenderBuffer->Bind();
        LightingPassFramebuffer->SetupDepthStencilAttachment(DepthStencilRenderBuffer);

        if (!LightingPassFramebuffer->IsComplete())
        {
            LUCID_LOG(ELogLevel::ERR, LUCID_TEXT("Failed to setup the lighting framebuffer"));
            return;
        }

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

        // Setup the framebuffer which will hold the final result
        FrameResultTexture = gpu::CreateEmpty2DTexture(ResultResolution.x,
                                                       ResultResolution.y,
                                                       gpu::ETextureDataType::FLOAT,
                                                       gpu::ETextureDataFormat::RGBA,
                                                       gpu::ETexturePixelFormat::RGBA,
                                                       0,
                                                       FSString{ "FrameResult" });
        FrameResultTexture->Bind();
        FrameResultTexture->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        FrameResultTexture->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

        FrameResultFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        FrameResultFramebuffer->SetupColorAttachment(0, FrameResultTexture);

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
            for (int i = 0; i < DEBUG_LINES_BUFFERS_COUNT; ++i)
            {
                gpu::FBufferDescription BufferDescription;
                BufferDescription.Data   = nullptr;
                BufferDescription.Offset = 0;
                BufferDescription.Size   = sizeof(glm::vec3) * 3 * MaxDebugLines;

                FDString BufferName        = SPrintf("DebugLinesVBO_%d", i);
                DebugLinesVertexBuffers[i] = gpu::CreateBuffer(BufferDescription, gpu::EBufferUsage::DYNAMIC, BufferName);

                DebugLinesFences[i] = gpu::CreateFence("");
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
        SkyboxPipelineState.Viewport = LightpassPipelineState.Viewport = PrepassPipelineState.Viewport = InRenderView->Viewport;

#if DEVELOPMENT
        GRenderStats.NumDrawCalls = 0;
        ++GRenderStats.FrameNumber;
        FrameTimer->StartTimer();
#endif

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
        DoGammaCorrection(LightingPassColorBuffer);
        gpu::PopDebugGroup();
#if DEVELOPMENT
        GRenderStats.FrameTimeMiliseconds = FrameTimer->EndTimer();
        RemoveStaleDebugLines();
#endif
    }

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
            for (int i = 0; i < InSceneToRender->StaticMeshes.GetLength(); ++i)
            {
                CStaticMesh* StaticMesh = InSceneToRender->StaticMeshes.GetByIndex(i);
                if (StaticMesh->bVisible)
                {
                    CurrentShadowMapShader->SetMatrix(MODEL_MATRIX, StaticMesh->CalculateModelMatrix());
#if DEVELOPMENT
                    if (resources::CMeshResource* MeshResource = StaticMesh->GetMeshResource())
                    {
                        gpu::PushDebugGroup(*MeshResource->GetName());
                        for (u16 SubMeshIdx = 0; SubMeshIdx < MeshResource->SubMeshes.GetLength(); ++SubMeshIdx)
                        {
                            MeshResource->SubMeshes[SubMeshIdx]->VAO->Bind();
                            MeshResource->SubMeshes[SubMeshIdx]->VAO->Draw();
                        }
                        gpu::PopDebugGroup();
                    }
                    else
                    {
                        LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a mesh resource", *StaticMesh->Name)
                        UnitCubeVAO->Bind();
                        UnitCubeVAO->Draw();
                    }
#else
                    for (u16 SubMeshIdx = 0; SubMeshIdx < StaticMesh->GetMeshResource()->SubMeshes.GetLength(); ++SubMeshIdx)
                    {
                        StaticMesh->GetMeshResource()->SubMeshes[SubMeshIdx]->VAO->Bind();
                        StaticMesh->GetMeshResource()->SubMeshes[SubMeshIdx]->VAO->Draw();
                    }
#endif
                }
            }

            gpu::PopDebugGroup();
        }
    }

    void CForwardRenderer::Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        // Z pre pass
        gpu::PushDebugGroup("Z pre pass");
        gpu::ConfigurePipelineState(PrepassPipelineState);

        BindAndClearFramebuffer(PrepassFramebuffer);
        PrepassFramebuffer->SetupDrawBuffers();

        PrepassShader->Use();
        SetupRendererWideUniforms(PrepassShader, InRenderView);

        const glm::vec2 NoiseTextureSize = { SSAONoise->GetWidth(), SSAONoise->GetHeight() };
        const glm::vec2 ViewportSize     = { InRenderView->Viewport.Width, InRenderView->Viewport.Height };
        const glm::vec2 NoiseScale       = ViewportSize / NoiseTextureSize;

        for (int i = 0; i < InSceneToRender->StaticMeshes.GetLength(); ++i)
        {
            CStaticMesh* StaticMesh = InSceneToRender->StaticMeshes.GetByIndex(i);
            if (StaticMesh->bVisible)
            {
                const glm::mat4 ModelMatrix = StaticMesh->CalculateModelMatrix();
                PrepassShader->SetMatrix(MODEL_MATRIX, ModelMatrix);
#if DEVELOPMENT
                if (resources::CMeshResource* MeshResource = StaticMesh->GetMeshResource())
                {
                    gpu::PushDebugGroup(*MeshResource->GetName());
                    for (u16 j = 0; j < MeshResource->SubMeshes.GetLength(); ++j)
                    {
                        const u16 MaterialIndex = MeshResource->SubMeshes[j]->MaterialIndex;
#if DEVELOPMENT
                        if (MaterialIndex >= StaticMesh->GetNumMaterialSlots() || StaticMesh->GetMaterialSlot(MaterialIndex) == nullptr)
                        {
                            LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a material at submesh %d", *StaticMesh->Name, j);
                            RenderWithDefaultMaterial(StaticMesh->MeshResource, j, nullptr, InRenderView, ModelMatrix);
                            PrepassShader->Use();
                            continue;
                        }
#endif
                        StaticMesh->GetMaterialSlot(MaterialIndex)->SetupShader(PrepassShader);

                        StaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Bind();
                        StaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Draw();
                    }
                    gpu::PopDebugGroup();
                }
                else
                {
                    LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a mesh resource", *StaticMesh->Name);
                    UnitCubeVAO->Bind();
                    UnitCubeVAO->Draw();
                }
#else
                for (u16 j = 0; j < StaticMesh->GetMeshResource()->SubMeshes.GetLength(); ++j)
                {
                    StaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Bind();
                    StaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Draw();
                }
#endif
            }
        }

        gpu::PopDebugGroup();

        // Calculate SSAO
        gpu::PushDebugGroup("SSAO");

        BindAndClearFramebuffer(SSAOFramebuffer);
        SSAOShader->Use();
        SetupRendererWideUniforms(SSAOShader, InRenderView);

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

        BindAndClearFramebuffer(LightingPassFramebuffer);
        LightingPassFramebuffer->SetupDrawBuffers();

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
            RenderWithoutLights(InScene, InRenderView);
            return;
        }

        gpu::CShader* LastUsedShader = nullptr;
        RenderLightContribution(&LastUsedShader, InScene->AllLights.GetByIndex(0), InScene, InRenderView);

        // Switch blending so we render the rest of the lights additively
        gpu::ConfigurePipelineState(LightpassPipelineState);
        for (int i = 1; i < InScene->AllLights.GetLength(); ++i)
        {
            RenderLightContribution(&LastUsedShader, InScene->AllLights.GetByIndex(i), InScene, InRenderView);
        }
    }

    void CForwardRenderer::RenderLightContribution(gpu::CShader**      LastShader,
                                                   const CLight*       InLight,
                                                   const FRenderScene* InScene,
                                                   const FRenderView*  InRenderView)
    {
        gpu::PushDebugGroup(*InLight->Name);
        for (u32 i = 0; i < InScene->StaticMeshes.GetLength(); ++i)
        {
            RenderStaticMesh(LastShader, InScene->StaticMeshes.GetByIndex(i), InLight, InRenderView);
        }
        gpu::PopDebugGroup();
    }

    void CForwardRenderer::RenderWithoutLights(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        gpu::CShader* LastUserShader = nullptr;

        for (int i = 0; i < InScene->StaticMeshes.GetLength(); ++i)
        {
            RenderStaticMesh(&LastUserShader, InScene->StaticMeshes.GetByIndex(i), nullptr, InRenderView);
        }
    }

    void CForwardRenderer::BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer)
    {
        InFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));
    }

    void CForwardRenderer::SetupRendererWideUniforms(gpu::CShader* InShader, const FRenderView* InRenderView)
    {
        InShader->SetFloat(AMBIENT_STRENGTH, AmbientStrength);
        InShader->SetInt(NUM_OF_PCF_SAMPLES, NumSamplesPCF);
        InShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());
        InShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        InShader->SetVector(VIEW_POSITION, InRenderView->Camera->Position);
        InShader->SetFloat(PARALLAX_HEIGHT_SCALE, 0.1f);
        InShader->SetVector(VIEWPORT_SIZE, glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height });
        InShader->UseTexture(AMBIENT_OCCLUSION, SSAOBlurred);
        InShader->SetFloat(NEAR_PLANE, InRenderView->Camera->NearPlane);
        InShader->SetFloat(FAR_PLANE, InRenderView->Camera->FarPlane);
    }

    void CForwardRenderer::RenderStaticMesh(gpu::CShader**     LastShader,
                                            const CStaticMesh* InStaticMesh,
                                            const CLight*      InLight,
                                            const FRenderView* InRenderView)
    {
        const glm::mat4 ModelMatrix = InStaticMesh->CalculateModelMatrix();

#if DEVELOPMENT
        if (InStaticMesh->GetMeshResource() == nullptr)
        {
            RenderWithDefaultMaterial(nullptr, -1, InLight, InRenderView, ModelMatrix);
            *LastShader = GEngine.GetDefaultMaterial()->GetShader();
            return;
        }
        gpu::PushDebugGroup(*InStaticMesh->Name);
#endif
        for (u16 j = 0; j < InStaticMesh->GetMeshResource()->SubMeshes.GetLength(); ++j)
        {

            const u16 MaterialIndex = InStaticMesh->GetMeshResource()->SubMeshes[j]->MaterialIndex;
#if DEVELOPMENT
            if (MaterialIndex >= InStaticMesh->GetNumMaterialSlots() || InStaticMesh->GetMaterialSlot(MaterialIndex) == nullptr)
            {
                LUCID_LOG(ELogLevel::ERR, "StaticMesh actor '%s' is missing a material at submesh %d", *InStaticMesh->Name, j);
                RenderWithDefaultMaterial(InStaticMesh->MeshResource, j, InLight, InRenderView, ModelMatrix);
                *LastShader = GEngine.GetDefaultMaterial()->GetShader();
                continue;
            }
#endif
            CMaterial* Material = InStaticMesh->GetMaterialSlot(MaterialIndex);

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms
            if (Material->GetShader() != *LastShader)
            {
                Material->GetShader()->Use();
                *LastShader = Material->GetShader();
                SetupRendererWideUniforms(*LastShader, InRenderView);
            }

            if (InLight)
            {
                InLight->SetupShader(*LastShader);
            }
            else
            {
                (*LastShader)->SetInt(LIGHT_TYPE, NO_LIGHT);
            }
            (*LastShader)->SetMatrix(MODEL_MATRIX, ModelMatrix);
            (*LastShader)->SetBool(REVERSE_NORMALS, InStaticMesh->GetReverseNormals());

            Material->SetupShader(*LastShader);

            InStaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Bind();
            InStaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Draw();
        }
        gpu::PopDebugGroup();
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
        HitMapShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());
        HitMapShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());

        // Render static geometry
        for (int i = 0; i < InScene->StaticMeshes.GetLength(); ++i)
        {
            CStaticMesh* StaticMesh = InScene->StaticMeshes.GetByIndex(i);
            if (StaticMesh->bVisible)
            {
                HitMapShader->SetUInt(ACTOR_ID, StaticMesh->Id);
                HitMapShader->SetMatrix(MODEL_MATRIX, StaticMesh->CalculateModelMatrix());
                if (StaticMesh->GetMeshResource())
                {
                    for (u16 j = 0; j < StaticMesh->GetMeshResource()->SubMeshes.GetLength(); ++j)
                    {
                        StaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Bind();
                        StaticMesh->GetMeshResource()->SubMeshes[j]->VAO->Draw();
                    }
                }
                else
                {
                    UnitCubeVAO->Bind();
                    UnitCubeVAO->Draw();
                }
            }
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
            BillboardHitMapShader->SetUInt(ACTOR_ID, Light->Id);

            ScreenWideQuadVAO->Draw();
        }

        // Get the result
        HitMapFramebuffer->ReadPixels(0, 0, HitMapTexture->GetWidth(), HitMapTexture->GetHeight(), CachedHitMap.CachedTextureData);
    }

    void CForwardRenderer::RenderWithDefaultMaterial(const resources::CMeshResource* InMeshResource,
                                                     const u16&                      InSubMeshIndex,
                                                     const CLight*                   InLight,
                                                     const FRenderView*              InRenderView,
                                                     const glm::mat4&                InModelMatrix)
    {
        CMaterial* DefaultMaterial = GEngine.GetDefaultMaterial();
        DefaultMaterial->GetShader()->Use();

        SetupRendererWideUniforms(DefaultMaterial->GetShader(), InRenderView);
        if (InLight)
        {
            InLight->SetupShader(DefaultMaterial->GetShader());
        }
        else
        {
            DefaultMaterial->GetShader()->SetInt(LIGHT_TYPE, NO_LIGHT);
        }
        DefaultMaterial->GetShader()->SetMatrix(MODEL_MATRIX, InModelMatrix);
        DefaultMaterial->SetupShader(DefaultMaterial->GetShader());

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

        SetupRendererWideUniforms(WorldGridShader, InRenderView);
        ScreenWideQuadVAO->Bind();
        ScreenWideQuadVAO->Draw();
    }

    void CForwardRenderer::RenderDebugLines(const FRenderView* InRenderView)
    {
        // Make sure that the buffer is no longer used
        int BufferIdx = GRenderStats.FrameNumber % DEBUG_LINES_BUFFERS_COUNT;

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
        SetupRendererWideUniforms(DebugLinesShader, InRenderView);

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
        DebugLinesFences[BufferIdx] = gpu::CreateFence("");
    }

#endif

    void CForwardRenderer::DoGammaCorrection(gpu::CTexture* InTexture)
    {
        BindAndClearFramebuffer(FrameResultFramebuffer);

        GammaCorrectionShader->Use();
        GammaCorrectionShader->SetFloat(GAMMA, Gamma);
        GammaCorrectionShader->UseTexture(SCENE_TEXTURE, InTexture);

        ScreenWideQuadVAO->Bind();
        ScreenWideQuadVAO->Draw();
    }

} // namespace lucid::scene