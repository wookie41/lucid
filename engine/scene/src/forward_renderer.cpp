#include "scene/forward_renderer.hpp"

#include "common/log.hpp"
#include "common/collections.hpp"

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/viewport.hpp"
#include "devices/gpu/cubemap.hpp"

#include "scene/lights.hpp"
#include "scene/blinn_phong_material.hpp"
#include "scene/camera.hpp"
#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"

#include "misc/basic_shapes.hpp"
#include "misc/math.hpp"

#include "resources/holder.hpp"

namespace lucid::scene
{
    static const u8 NO_LIGHT = 0;
    static const FString LIGHT_TYPE("uLight.Type");

    static const FString VIEWPORT_SIZE("uViewportSize");

    static const FString SSAO_POSITIONS_VS("uPositionsVS");
    static const FString SSAO_NORMALS_VS("uNormalsVS");
    static const FString SSAO_NOISE("uNoise");
    static const FString SSAO_NOISE_SCALE("uNoiseScale");
    static const FString SSAO_RADIUS("uRadius");
    static const FString SSAO_BIAS("uBias");

    static const FString SIMPLE_BLUR_OFFSET_X("uOffsetX");
    static const FString SIMPLE_BLUR_OFFSET_Y("uOffsetY");
    static const FString SIMPLE_BLUR_TEXTURE("uTextureToBlur");

    // Shader-wide uniforms
    static const FString AMBIENT_STRENGTH("uAmbientStrength");
    static const FString NUM_OF_PCF_SAMPLES("uNumSamplesPCF");
    static const FString AMBIENT_OCCLUSION("uAmbientOcclusion");

    static const FString VIEW_POSITION("uViewPos");

    static const FString MODEL_MATRIX("uModel");
    static const FString REVERSE_NORMALS("uReverseNormals");
    static const FString VIEW_MATRIX("uView");
    static const FString PROJECTION_MATRIX("uProjection");

    static const FString SKYBOX_CUBEMAP("uSkybox");

    static const FString PARALLAX_HEIGHT_SCALE("uParallaxHeightScale");

#if DEVELOPMENT
    static const FString RENDERABLE_ID("uRenderableId");
#endif

    
    ForwardRenderer::ForwardRenderer(const u32& InMaxNumOfDirectionalLights,
                                     const u8& InNumSSAOSamples,
                                     gpu::CShader* InShadowMapShader,
                                     gpu::CShader* InShadowCubeMapShader,
                                     gpu::CShader* InPrepassShader,
                                     gpu::CShader* InSSAOShader,
                                     gpu::CShader* InSimpleBlurShader,
                                     gpu::CShader* InSkyboxShader,
                                     gpu::CShader* InHitMapShader,
                                     gpu::CVertexArray* InScreenWideQuadVAO,
                                     gpu::CVertexArray* InUnitCubeVAO)
    : MaxNumOfDirectionalLights(InMaxNumOfDirectionalLights), NumSSAOSamples(InNumSSAOSamples),
      ShadowMapShader(InShadowMapShader), ShadowCubeMapShader(InShadowCubeMapShader), PrepassShader(InPrepassShader),
      SSAOShader(InSSAOShader), SimpleBlurShader(InSimpleBlurShader), SkyboxShader(InSkyboxShader), HitMapShader(InHitMapShader),
      ScreenWideQuadVAO(InScreenWideQuadVAO), UnitCubeVAO(InUnitCubeVAO)
    {
    }

    void ForwardRenderer::Setup()
    {
        if (!ScreenWideQuadVAO)
        {
            ScreenWideQuadVAO = misc::CreateQuadVAO();
        }

        if (!UnitCubeVAO)
        {
            UnitCubeVAO = misc::CreateCubeVAO();
        }

        // Prepare pipeline states
        ShadowMapGenerationPipelineState.ClearColorBufferColor = FColor{ 0 };
        ShadowMapGenerationPipelineState.ClearDepthBufferValue = 0;
        ShadowMapGenerationPipelineState.IsDepthTestEnabled = true;
        ShadowMapGenerationPipelineState.DepthTestFunction = gpu::EDepthTestFunction::LEQUAL;
        ShadowMapGenerationPipelineState.IsBlendingEnabled = false;
        ShadowMapGenerationPipelineState.IsCullingEnabled = false;
        ShadowMapGenerationPipelineState.IsSRGBFramebufferEnabled = false;
        ShadowMapGenerationPipelineState.IsDepthBufferReadOnly = false;

        PrepassPipelineState.ClearColorBufferColor = FColor{ 0 };
        PrepassPipelineState.IsDepthTestEnabled = true;
        PrepassPipelineState.DepthTestFunction = gpu::EDepthTestFunction::LEQUAL;
        PrepassPipelineState.IsBlendingEnabled = false;
        PrepassPipelineState.IsCullingEnabled = false;
        // @TODO
        // PrepassPipelineState.IsCullingEnabled = false;
        // PrepassPipelineState.CullMode = gpu::ECullMode::BACK;
        PrepassPipelineState.IsSRGBFramebufferEnabled = true;
        PrepassPipelineState.IsDepthBufferReadOnly = false;
        PrepassPipelineState.ClearDepthBufferValue = 0.f;

        InitialLightLightpassPipelineState.ClearColorBufferColor = FColor{ 0 };
        InitialLightLightpassPipelineState.IsDepthTestEnabled = true;
        InitialLightLightpassPipelineState.DepthTestFunction = gpu::EDepthTestFunction::EQUAL;
        InitialLightLightpassPipelineState.IsBlendingEnabled = true;
        InitialLightLightpassPipelineState.BlendFunctionSrc = gpu::EBlendFunction::ONE;
        InitialLightLightpassPipelineState.BlendFunctionDst = gpu::EBlendFunction::ZERO;
        InitialLightLightpassPipelineState.BlendFunctionAlphaSrc = gpu::EBlendFunction::ONE;
        InitialLightLightpassPipelineState.BlendFunctionAlphaDst = gpu::EBlendFunction::ZERO;
        // @TODO
        InitialLightLightpassPipelineState.IsCullingEnabled = false;
        // InitialLightLightpassPipelineState.IsCullingEnabled = true;
        // InitialLightLightpassPipelineState.CullMode = gpu::ECullMode::BACK;
        InitialLightLightpassPipelineState.IsSRGBFramebufferEnabled = true;
        InitialLightLightpassPipelineState.IsDepthBufferReadOnly = true;
        InitialLightLightpassPipelineState.ClearDepthBufferValue = 0.f;

        LightpassPipelineState = InitialLightLightpassPipelineState;
        LightpassPipelineState.BlendFunctionDst = gpu::EBlendFunction::ONE;
        LightpassPipelineState.BlendFunctionAlphaDst = gpu::EBlendFunction::ONE;

        SkyboxPipelineState = LightpassPipelineState;
        SkyboxPipelineState.IsBlendingEnabled = false;
        SkyboxPipelineState.DepthTestFunction = gpu::EDepthTestFunction::LEQUAL;

        HitMapGenerationPipelineState = SkyboxPipelineState;
        HitMapGenerationPipelineState.IsDepthBufferReadOnly = false;
        HitMapGenerationPipelineState.IsSRGBFramebufferEnabled = false;

        // Create the framebuffers
        ShadowMapFramebuffer = gpu::CreateFramebuffer(FString{ "ShadowmapFramebuffer" });
        PrepassFramebuffer = gpu::CreateFramebuffer(FString{ "PrepassFramebuffer" });
        LightingPassFramebuffer = gpu::CreateFramebuffer(FString{ "LightingPassFramebuffer" });
        SSAOFramebuffer = gpu::CreateFramebuffer(FString{ "SSAOFramebuffer" });
        BlurFramebuffer = gpu::CreateFramebuffer(FString{ "BlueFramebuffer" });

        // Create a common depth-stencil attachment for both framebuffers
        DepthStencilRenderBuffer = gpu::CreateRenderbuffer(
          gpu::ERenderbufferFormat::DEPTH24_STENCIL8, FramebufferSize, FString{ "LightingPassRenderbuffer" });

        // Create render targets in which we'll store some additional information during the depth prepass
        CurrentFrameVSNormalMap = gpu::CreateEmpty2DTexture(FramebufferSize.x,
                                                            FramebufferSize.y,
                                                            gpu::ETextureDataType::FLOAT,
                                                            gpu::ETextureDataFormat::RGB16F,
                                                            gpu::ETexturePixelFormat::RGB,
                                                            0,
                                                            FString{ "CurrentFrameVSNormalMap" });
        CurrentFrameVSPositionMap = gpu::CreateEmpty2DTexture(FramebufferSize.x,
                                                              FramebufferSize.y,
                                                              gpu::ETextureDataType::FLOAT,
                                                              gpu::ETextureDataFormat::RGB16F,
                                                              gpu::ETexturePixelFormat::RGB,
                                                              0,
                                                              FString{ "CurrentFrameVSPositionMap" });

        // Setup the prepass framebuffer
        PrepassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        CurrentFrameVSNormalMap->Bind();
        CurrentFrameVSNormalMap->SetMinFilter(gpu::MinTextureFilter::NEAREST);
        CurrentFrameVSNormalMap->SetMagFilter(gpu::MagTextureFilter::NEAREST);
        PrepassFramebuffer->SetupColorAttachment(0, CurrentFrameVSNormalMap);

        CurrentFrameVSPositionMap->Bind();
        CurrentFrameVSPositionMap->SetMinFilter(gpu::MinTextureFilter::NEAREST);
        CurrentFrameVSPositionMap->SetMagFilter(gpu::MagTextureFilter::NEAREST);
        PrepassFramebuffer->SetupColorAttachment(1, CurrentFrameVSPositionMap);

        DepthStencilRenderBuffer->Bind();
        PrepassFramebuffer->SetupDepthStencilAttachment(DepthStencilRenderBuffer);

        if (!PrepassFramebuffer->IsComplete())
        {
            LUCID_LOG(ELogLevel::ERR, LUCID_TEXT("Failed to setup the prepass framebuffer"));
            return;
        }

        // Create texture to store SSO result
        SSAOResult = gpu::CreateEmpty2DTexture(FramebufferSize.x,
                                               FramebufferSize.y,
                                               gpu::ETextureDataType::FLOAT,
                                               gpu::ETextureDataFormat::R,
                                               gpu::ETexturePixelFormat::RED,
                                               0,
                                               FString{ "SSAOResult" });
        SSAOResult->Bind();
        SSAOResult->SetMinFilter(gpu::MinTextureFilter::NEAREST);
        SSAOResult->SetMagFilter(gpu::MagTextureFilter::NEAREST);

        // Create texture for the blurred SSAO result
        SSAOBlurred = gpu::CreateEmpty2DTexture(FramebufferSize.x,
                                                FramebufferSize.y,
                                                gpu::ETextureDataType::FLOAT,
                                                gpu::ETextureDataFormat::R,
                                                gpu::ETexturePixelFormat::RED,
                                                0,
                                                FString{ "SSOBlurred" });
        SSAOBlurred->Bind();
        SSAOBlurred->SetMinFilter(gpu::MinTextureFilter::NEAREST);
        SSAOBlurred->SetMagFilter(gpu::MagTextureFilter::NEAREST);

        // Setup a SSAO framebuffer
        SSAOFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        SSAOFramebuffer->SetupColorAttachment(0, SSAOResult);

        if (!SSAOFramebuffer->IsComplete())
        {
            LUCID_LOG(ELogLevel::ERR, LUCID_TEXT("Failed to setup the SSAO framebuffer"));
            return;
        }

        // Create color attachment for the lighting pass framebuffer
        LightingPassColorBuffer = gpu::CreateEmpty2DTexture(FramebufferSize.x,
                                                            FramebufferSize.y,
                                                            gpu::ETextureDataType::FLOAT,
                                                            gpu::ETextureDataFormat::RGBA,
                                                            gpu::ETexturePixelFormat::RGBA,
                                                            0,
                                                            FString{ "LightingPassColorBuffer" });

        // Setup the lighting pass framebuffer
        LightingPassFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        LightingPassColorBuffer->Bind();
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
            Sample = glm::normalize(Sample);
            Sample *= math::RandomFloat();

            // Use an accelerating interpolation function so there are more samples close to the fragment
            float Scale = (float)i / (float)NumSSAOSamples;
            Scale = math::Lerp(0.1, 1.0f, Scale * Scale);
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
            Noise[i] = math::RandomVec2();
            Noise[i].x = Noise[i].x * 2.0 - 1.0;
            Noise[i].y = Noise[i].y * 2.0 - 1.0;
        }

        SSAONoise = gpu::Create2DTexture(Noise,
                                         4,
                                         4,
                                         gpu::ETextureDataType::FLOAT,
                                         gpu::ETextureDataFormat::RG32F,
                                         gpu::ETexturePixelFormat::RG,
                                         0,
                                         FString{ "SSAONoise" });
        SSAONoise->Bind();
        SSAONoise->SetWrapSFilter(gpu::WrapTextureFilter::REPEAT);
        SSAONoise->SetWrapTFilter(gpu::WrapTextureFilter::REPEAT);
        SSAOShader->UseTexture(SSAO_NOISE, SSAONoise);
        SSAOShader->SetFloat(SSAO_RADIUS, SSAORadius);

        // HitMap generation
#if DEVELOPMENT
        HitMapTexture = gpu::CreateEmpty2DTexture(FramebufferSize.x,
                                                  FramebufferSize.y,
                                                  gpu::ETextureDataType::UNSIGNED_INT,
                                                  gpu::ETextureDataFormat::R32UI,
                                                  gpu::ETexturePixelFormat::RED_INTEGER,
                                                  0,
                                                  FString{ "HitMapTexture" });


        HitMapDepthStencilRenderbuffer = gpu::CreateRenderbuffer(gpu::ERenderbufferFormat::DEPTH24_STENCIL8, { FramebufferSize.x, FramebufferSize.y }, FString{ "HitMapRenderbuffer" });

        HitMapFramebuffer = gpu::CreateFramebuffer(FString{ "HitMapMapFramebuffer" });
        HitMapFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        HitMapTexture->Bind();
        HitMapTexture->SetMinFilter(gpu::MinTextureFilter::NEAREST);
        HitMapTexture->SetMagFilter(gpu::MagTextureFilter::NEAREST);

        HitMapFramebuffer->SetupColorAttachment(0, HitMapTexture);

        HitMapDepthStencilRenderbuffer->Bind();
        HitMapFramebuffer->SetupDepthStencilAttachment(HitMapDepthStencilRenderbuffer);

        HitMapFramebuffer->IsComplete();

        CachedHitMap.Width = FramebufferSize.x;
        CachedHitMap.Height = FramebufferSize.y;
        CachedHitMap.CachedTextureData = (u32*)malloc(HitMapTexture->GetSizeInBytes()); // @Note doesn't get freed, but it's probably okay as it should die with the editor
        Zero(CachedHitMap.CachedTextureData, HitMapTexture->GetSizeInBytes());
#endif
    }

    void ForwardRenderer::Cleanup()
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

    void ForwardRenderer::Render(FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        SkyboxPipelineState.Viewport = LightpassPipelineState.Viewport = PrepassPipelineState.Viewport = InRenderView->Viewport;

        gpu::SetViewport(InRenderView->Viewport);

        GenerateShadowMaps(InSceneToRender);
        Prepass(InSceneToRender, InRenderView);
        LightingPass(InSceneToRender, InRenderView);
#if DEVELOPMENT
        GenerateHitmap(InSceneToRender, InRenderView);
#endif
    }

    void ForwardRenderer::GenerateShadowMaps(FRenderScene* InSceneToRender)
    {
        // Prepare the pipeline state
        gpu::ConfigurePipelineState(ShadowMapGenerationPipelineState);

        ShadowMapFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        ShadowMapFramebuffer->DisableReadWriteBuffers();

        u8 PrevShadowMapQuality = 255;
        gpu::CShader* PrevShadowMapShader = nullptr;

        for (int i = 0; i < arrlen(InSceneToRender->Lights); ++i)
        {
            CLight* Light = InSceneToRender->Lights[i];            

            // @TODO This should happen only if light moves
            Light->UpdateLightSpaceMatrix(LightSettingsByQuality[Light->Quality]);

            gpu::CShader* CurrentShadowMapShader = Light->GetType() == ELightType::POINT ? ShadowCubeMapShader : ShadowMapShader;
            if (CurrentShadowMapShader != PrevShadowMapShader)
            {
                PrevShadowMapShader = CurrentShadowMapShader;
                CurrentShadowMapShader->Use();
            }

            Light->SetupShadowMapShader(CurrentShadowMapShader);

            // Check if we need to adjust the viewport based on light's shadow map quality
            if (Light->ShadowMap->GetQuality() != PrevShadowMapQuality)
            {
                PrevShadowMapQuality = Light->ShadowMap->GetQuality();
                gpu::SetViewport({ 0,
                                   0,
                                   (u32)ShadowMapSizeByQuality[PrevShadowMapQuality].x,
                                   (u32)ShadowMapSizeByQuality[PrevShadowMapQuality].y });
            }

            // Setup light's shadow map texture
            ShadowMapFramebuffer->SetupDepthAttachment(Light->ShadowMap->GetShadowMapTexture());
            gpu::ClearBuffers(gpu::EGPUBuffer::DEPTH);

            // Static geometry
            for (int i = 0; i < arrlen(InSceneToRender->StaticMeshes); ++i)
            {
                CStaticMesh* StaticMesh = InSceneToRender->StaticMeshes[i];
                CurrentShadowMapShader->SetMatrix(MODEL_MATRIX, StaticMesh->CalculateModelMatrix());
                StaticMesh->GetVertexArray()->Bind();
                StaticMesh->GetVertexArray()->Draw();
            }
        }
    }

    void ForwardRenderer::Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        // Render the scene
        gpu::ConfigurePipelineState(PrepassPipelineState);

        BindAndClearFramebuffer(PrepassFramebuffer);
        PrepassFramebuffer->SetupDrawBuffers();

        PrepassShader->Use();
        SetupRendererWideUniforms(PrepassShader, InRenderView);

        const glm::vec2 NoiseTextureSize = { SSAONoise->GetSize().x, SSAONoise->GetSize().y };
        const glm::vec2 ViewportSize = { InRenderView->Viewport.Width, InRenderView->Viewport.Height };
        const glm::vec2 NoiseScale = ViewportSize / NoiseTextureSize;

        for (int i = 0; i < arrlen(InSceneToRender->StaticMeshes); ++i)
        {
            RenderStaticMesh(PrepassShader, InSceneToRender->StaticMeshes[i]);
        }

        // Calculate SSAO
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
        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));

        SimpleBlurShader->Use();
        SimpleBlurShader->UseTexture(SIMPLE_BLUR_TEXTURE, SSAOResult);
        SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_X, SimpleBlurXOffset);
        SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_Y, SimpleBlurYOffset);

        ScreenWideQuadVAO->Bind();
        ScreenWideQuadVAO->Draw();
    }

    void ForwardRenderer::LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
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

    void ForwardRenderer::RenderStaticMeshes(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        if (arrlen(InScene->Lights) == 0)
        {
            RenderWithoutLights(InScene, InRenderView);
            return;
        }

        RenderLightContribution(InScene->Lights[0], InScene, InRenderView);

        // Switch blending so we render the rest of the lights additively
        gpu::ConfigurePipelineState(LightpassPipelineState);
        for (int i = 1; i < arrlen(InScene->Lights); ++i)
        {
            RenderLightContribution(InScene->Lights[i], InScene, InRenderView);            
        }
    }

    void
    ForwardRenderer::RenderLightContribution(const CLight* InLight, const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        gpu::CShader* LastUsedShader = nullptr;
        for (int i = 0; i < arrlen(InScene->StaticMeshes); ++i)
        {
            CStaticMesh* CurrentMesh = InScene->StaticMeshes[i];

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms
            auto Shader = CurrentMesh->GetMaterial()->GetShader();
            if (Shader != LastUsedShader)
            {
                Shader->Use();
            }
            LastUsedShader = Shader;

            SetupRendererWideUniforms(LastUsedShader, InRenderView);
            InLight->SetupShader(Shader);
            RenderStaticMesh(Shader, CurrentMesh);
        }
    }

    void ForwardRenderer::RenderWithoutLights(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        gpu::CShader* LastUserShader = nullptr;

        for (int i = 0; i < arrlen(InScene->StaticMeshes); ++i)
        {
            CStaticMesh* StaticMesh = InScene->StaticMeshes[i];

            // Determine if the material uses a custom shader if yes, then setup the renderer-provided uniforms
            gpu::CShader* CustomShader = StaticMesh->GetMaterial()->GetShader();
            if (CustomShader)
            {
                CustomShader->Use();
                CustomShader->SetInt(LIGHT_TYPE, NO_LIGHT);
                SetupRendererWideUniforms(CustomShader, InRenderView);
                LastUserShader = CustomShader;
            }
            else if (LastUserShader != CustomShader)
            {
                CustomShader->Use();
                LastUserShader = CustomShader;
            }

            RenderStaticMesh(LastUserShader, StaticMesh);
        }
    }

    void ForwardRenderer::BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer)
    {
        InFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));
    }

    void ForwardRenderer::SetupRendererWideUniforms(gpu::CShader* InShader, const FRenderView* InRenderView)
    {
        InShader->SetFloat(AMBIENT_STRENGTH, AmbientStrength);
        InShader->SetInt(NUM_OF_PCF_SAMPLES, NumSamplesPCF);
        InShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());
        InShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        InShader->SetVector(VIEW_POSITION, InRenderView->Camera->Position);
        InShader->SetFloat(PARALLAX_HEIGHT_SCALE, 0.1f);
        InShader->SetVector(VIEWPORT_SIZE, glm::vec2{ InRenderView->Viewport.Width, InRenderView->Viewport.Height });
        InShader->UseTexture(AMBIENT_OCCLUSION, SSAOBlurred);
    }

    void ForwardRenderer::RenderStaticMesh(gpu::CShader* Shader, const CStaticMesh* InStaticMesh)
    {
        const glm::mat4 ModelMatrix = InStaticMesh->CalculateModelMatrix();
        Shader->SetMatrix(MODEL_MATRIX, ModelMatrix);
        Shader->SetBool(REVERSE_NORMALS, InStaticMesh->GetReverseNormals());

        InStaticMesh->GetMaterial()->SetupShader(Shader);

        InStaticMesh->GetVertexArray()->Bind();
        InStaticMesh->GetVertexArray()->Draw();
    }

    inline void ForwardRenderer::RenderSkybox(const CSkybox* InSkybox, const FRenderView* InRenderView)
    {
        gpu::ConfigurePipelineState(SkyboxPipelineState);

        SkyboxShader->Use();

        SkyboxShader->UseTexture(SKYBOX_CUBEMAP, InSkybox->GetCubemap());
        SkyboxShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        SkyboxShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());

        UnitCubeVAO->Bind();
        UnitCubeVAO->Draw();
    }

#if DEVELOPMENT
    void ForwardRenderer::GenerateHitmap(const FRenderScene* InScene, const FRenderView* InRenderView) const
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
        for (int i = 0; i < arrlen(InScene->StaticMeshes); ++i)
        {
            CStaticMesh* StaticMesh = InScene->StaticMeshes[i];
            HitMapShader->SetUInt(RENDERABLE_ID, StaticMesh->Id);
            HitMapShader->SetMatrix(MODEL_MATRIX, StaticMesh->CalculateModelMatrix());
            StaticMesh->GetVertexArray()->Bind();
            StaticMesh->GetVertexArray()->Draw();
        }

        // Get the result
        HitMapFramebuffer->ReadPixels(CachedHitMap.CachedTextureData);
    }
#endif

} // namespace lucid::scene	