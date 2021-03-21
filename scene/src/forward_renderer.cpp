#include "scene/forward_renderer.hpp"

#include "common/log.hpp"
#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"
#include "scene/lights.hpp"

#include "scene/renderable.hpp"
#include "scene/blinn_phong_material.hpp"

#include "devices/gpu/gpu.hpp"

#include "devices/gpu/cubemap.hpp"
#include "devices/gpu/viewport.hpp"

#include "misc/basic_shapes.hpp"
#include "misc/math.hpp"

#include "common/collections.hpp"
#include "scene/camera.hpp"

namespace lucid::scene
{
    static const u32 NO_LIGHT = 0;
    static const u32 DIRECTIONAL_LIGHT = 1;
    static const u32 POINT_LIGHT = 2;
    static const u32 SPOT_LIGHT = 3;

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
    
    static const FString LIGHT_TYPE("uLight.Type");
    static const FString LIGHT_POSITION("uLight.Position");
    static const FString LIGHT_DIRECTION("uLight.Direction");
    static const FString LIGHT_COLOR("uLight.Color");
    static const FString LIGHT_CONSTANT("uLight.Constant");
    static const FString LIGHT_LINEAR("uLight.Linear");
    static const FString LIGHT_QUADRATIC("uLight.Quadratic");
    static const FString LIGHT_INNER_CUT_OFF("uLight.InnerCutOffCos");
    static const FString LIGHT_OUTER_CUT_OFF("uLight.OuterCutOffCos");
    static const FString LIGHT_SHADOW_MAP("uLight.ShadowMap");
    static const FString LIGHT_SPACE_MATRIX("uLight.LightSpaceMatrix");
    static const FString LIGHT_CASTS_SHADOWS("uLight.CastsShadows");
    static const FString LIGHT_FAR_PLANE("uLight.FarPlane");
    static const FString LIGHT_SHADOW_CUBE("uShadowCube");

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

    ForwardRenderer::ForwardRenderer(
        const u32& InMaxNumOfDirectionalLights,
        const u8 InNumSSAOSamples,
        gpu::CShader* InDefaultRenderableShader,
        gpu::CShader* InPrepassShader,
        gpu::CShader* InSSAOShader,
        gpu::CShader* InSimpleBlurShader,
        gpu::CShader* InSkyboxShader)
    : CRenderer(InDefaultRenderableShader),
        MaxNumOfDirectionalLights(InMaxNumOfDirectionalLights),
        NumSSAOSamples(InNumSSAOSamples),
        PrepassShader(InPrepassShader),
        SSAOShader(InSSAOShader),
        SimpleBlurShader(InSimpleBlurShader),
        SkyboxShader(InSkyboxShader)
    {
    }

    void ForwardRenderer::Setup()
    {
        // Create the framebuffers
        PrepassFramebuffer = gpu::CreateFramebuffer();
        LightingPassFramebuffer = gpu::CreateFramebuffer();
        SSAOFramebuffer = gpu::CreateFramebuffer();
        BlurFramebuffer = gpu::CreateFramebuffer();

        // Create a common depth-stencil attachment for both framebuffers
        DepthStencilRenderBuffer = gpu::CreateRenderbuffer(gpu::ERenderbufferFormat::DEPTH24_STENCIL8, FramebufferSize);

        // Create render targets in which we'll store some additional information during the depth prepass
        CurrentFrameVSNormalMap = gpu::CreateEmpty2DTexture(FramebufferSize.x, FramebufferSize.y, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::RGB16F, gpu::ETexturePixelFormat::RGB, 0);
        CurrentFrameVSPositionMap = gpu::CreateEmpty2DTexture(FramebufferSize.x, FramebufferSize.y, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::RGB16F, gpu::ETexturePixelFormat::RGB, 0);
        
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
        SSAOResult = gpu::CreateEmpty2DTexture(FramebufferSize.x ,FramebufferSize.y, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::R, gpu::ETexturePixelFormat::R, 0);
        SSAOResult->Bind();
        SSAOResult->SetMinFilter(gpu::MinTextureFilter::NEAREST);
        SSAOResult->SetMagFilter(gpu::MagTextureFilter::NEAREST);

        // Create texture for the blurred SSAO result
        SSAOBlurred = gpu::CreateEmpty2DTexture(FramebufferSize.x ,FramebufferSize.y, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::R, gpu::ETexturePixelFormat::R, 0);
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
        LightingPassColorBuffer = gpu::CreateEmpty2DTexture(FramebufferSize.x, FramebufferSize.y, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::RGBA, gpu::ETexturePixelFormat::RGBA, 0);

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
            float Scale = (float)i/(float)NumSSAOSamples;
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

        SSAONoise = gpu::Create2DTexture(Noise, 4, 4, gpu::ETextureDataType::FLOAT, gpu::ETextureDataFormat::RG32F, gpu::ETexturePixelFormat::RG, 0);
        SSAONoise->Bind();
        SSAONoise->SetWrapSFilter(gpu::WrapTextureFilter::REPEAT);
        SSAONoise->SetWrapTFilter(gpu::WrapTextureFilter::REPEAT);
        SSAOShader->UseTexture(SSAO_NOISE, SSAONoise);
        SSAOShader->SetFloat(SSAO_RADIUS, SSAORadius);
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

    void ForwardRenderer::Render(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        gpu::EnableDepthTest();
        gpu::DisableSRGBFramebuffer();
        gpu::SetViewport(InRenderView->Viewport);

        Prepass(InSceneToRender, InRenderView);
        LightingPass(InSceneToRender, InRenderView);
    }

    void ForwardRenderer::RenderStaticGeometry(const FRenderScene* InScene, const FRenderView* InRenderView)
    {
        gpu::SetCullMode(gpu::CullMode::BACK);

        // Render with lights contribution, ie. update the lighting uniforms,
        // as the underlying shader will use them when rendering the geometry

        gpu::EnableBlending();
        gpu::SetBlendFunctionSeparate(gpu::BlendFunction::ONE, gpu::BlendFunction::ZERO, gpu::BlendFunction::ONE,
                                      gpu::BlendFunction::ZERO);

        const FLinkedListItem<CLight>* LightNode = &InScene->Lights.Head;
        if (!LightNode->Element)
        {
            // no lights in the scene, render the geometry only with ambient contribution
            RenderWithoutLights(InScene, InRenderView);
            return;
        }

        RenderLightContribution(LightNode->Element, InScene, InRenderView);
        if (!LightNode->Next || !LightNode->Next->Element)
        {
            // No more lights
            return;
        }

        // Change the blending mode so we render the rest of the lights additively
        gpu::SetBlendFunction(gpu::BlendFunction::ONE, gpu::BlendFunction::ONE);

        LightNode = LightNode->Next;
        while (LightNode && LightNode->Element)
        {
            RenderLightContribution(LightNode->Element, InScene, InRenderView);
            LightNode = LightNode->Next;
        }
    }

    void ForwardRenderer::RenderLightContribution(const CLight* InLight,
                                                  const FRenderScene* InScene,
                                                  const FRenderView* InRenderView)
    {
        const FLinkedListItem<FRenderable>* CurrentNode = &InScene->StaticGeometry.Head;
        gpu::CShader* LastUsedShader = DefaultRenderableShader;
        while (CurrentNode && CurrentNode->Element)
        {
            FRenderable* CurrentRenderable = CurrentNode->Element;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms
            auto CustomShader = CurrentRenderable->Material->GetCustomShader();
            LastUsedShader = CustomShader ? CustomShader : DefaultRenderableShader;

            LastUsedShader->Use();
            SetupRendererWideUniforms(LastUsedShader, InRenderView);
            SetupLight(LastUsedShader, InLight);
            Render(LastUsedShader, CurrentRenderable);

            CurrentNode = CurrentNode->Next;
        }
    }

    void ForwardRenderer::SetupLight(gpu::CShader* InShader, const CLight* InLight)
    {
        InShader->SetVector(LIGHT_COLOR, InLight->Color);
        InShader->SetVector(LIGHT_POSITION, InLight->Position);

        switch (InLight->GetType())
        {
        case ELightType::DIRECTIONAL:
        {
            CDirectionalLight* light = (CDirectionalLight*)InLight;
            InShader->SetVector(LIGHT_DIRECTION, light->Direction);
            InShader->SetInt(LIGHT_TYPE, DIRECTIONAL_LIGHT);
            InShader->SetMatrix(LIGHT_SPACE_MATRIX, light->LightSpaceMatrix);

            if (light->ShadowMap != nullptr)
            {
                InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
                InShader->UseTexture(LIGHT_SHADOW_MAP, light->ShadowMap);
            }
            else
            {
                InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
            }
            break;
        }
        case ELightType::POINT:
        {
            CPointLight* light = (CPointLight*)InLight;
            InShader->SetFloat(LIGHT_CONSTANT, light->Constant);
            InShader->SetFloat(LIGHT_LINEAR, light->Linear);
            InShader->SetFloat(LIGHT_QUADRATIC, light->Quadratic);
            InShader->SetFloat(LIGHT_FAR_PLANE, light->FarPlane);
            InShader->SetInt(LIGHT_TYPE, POINT_LIGHT);

            if (light->ShadowMap != nullptr)
            {
                InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
                InShader->UseTexture(LIGHT_SHADOW_CUBE, light->ShadowMap);
            }
            else
            {
                InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
            }

            break;
        }
        case ELightType::SPOT:
        {
            CSpotLight* light = (CSpotLight*)InLight;
            InShader->SetVector(LIGHT_DIRECTION, light->Direction);
            InShader->SetFloat(LIGHT_CONSTANT, light->Constant);
            InShader->SetFloat(LIGHT_LINEAR, light->Linear);
            InShader->SetFloat(LIGHT_QUADRATIC, light->Quadratic);
            InShader->SetFloat(LIGHT_INNER_CUT_OFF, glm::cos(light->InnerCutOffRad));
            InShader->SetFloat(LIGHT_OUTER_CUT_OFF, glm::cos(light->OuterCutOffRad));
            InShader->SetInt(LIGHT_TYPE, SPOT_LIGHT);
            InShader->SetMatrix(LIGHT_SPACE_MATRIX, light->LightSpaceMatrix);

            if (light->ShadowMap != nullptr)
            {
                InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
                InShader->UseTexture(LIGHT_SHADOW_MAP, light->ShadowMap);
            }
            else
            {
                InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
            }
            break;
        }
        }
    }

    void ForwardRenderer::RenderWithoutLights(const FRenderScene* InScene,
                                              const FRenderView* InRenderView)
    {
        const FLinkedListItem<FRenderable>* CurrentNode = &InScene->StaticGeometry.Head;
        gpu::CShader* LastUserShader = DefaultRenderableShader;
        
        LastUserShader->SetInt(LIGHT_TYPE, NO_LIGHT);

        while (CurrentNode && CurrentNode->Element)
        {
            FRenderable* CurrentRenderable = CurrentNode->Element;

            // Determine if the material uses a custom shader if yes, then setup the renderer-provided uniforms
            gpu::CShader* CustomShader = CurrentRenderable->Material->GetCustomShader();
            if (CustomShader)
            {
                CustomShader->Use();
                CustomShader->SetInt(LIGHT_TYPE, NO_LIGHT);
                SetupRendererWideUniforms(CustomShader, InRenderView);
                LastUserShader = CustomShader;
            }
            else if (LastUserShader != DefaultRenderableShader)
            {
                DefaultRenderableShader->Use();
                LastUserShader = DefaultRenderableShader;
            }

            Render(LastUserShader, CurrentRenderable);
            CurrentNode = CurrentNode->Next;
        }
    }

    void ForwardRenderer::Prepass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        // Render the scene
        gpu::SetReadOnlyDepthBuffer(false);
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        BindAndClearFramebuffer(PrepassFramebuffer);
        PrepassFramebuffer->SetupDrawBuffers();
        
        PrepassShader->Use();
        SetupRendererWideUniforms(PrepassShader, InRenderView);

        const glm::vec2 NoiseTextureSize = { SSAONoise->GetSize().x, SSAONoise->GetSize().y };
        const glm::vec2 ViewportSize = { InRenderView->Viewport.Width, InRenderView->Viewport.Height };
        const glm::vec2 NoiseScale = ViewportSize / NoiseTextureSize;

        const FLinkedListItem<FRenderable>* CurrentNode = &InSceneToRender->StaticGeometry.Head;
        while (CurrentNode && CurrentNode->Element)
        {
            FRenderable* CurrentRenderable = CurrentNode->Element;
            Render(PrepassShader, CurrentRenderable);
            CurrentNode = CurrentNode->Next;
        }

         // Calculate SSAO
        gpu::DisableDepthTest();
        BindAndClearFramebuffer(SSAOFramebuffer);
        SSAOShader->Use();
        SetupRendererWideUniforms(SSAOShader, InRenderView);

        SSAOShader->UseTexture(SSAO_POSITIONS_VS, CurrentFrameVSPositionMap);
        SSAOShader->UseTexture(SSAO_NORMALS_VS, CurrentFrameVSNormalMap);
        SSAOShader->UseTexture(SSAO_NOISE, SSAONoise);
        SSAOShader->SetVector(SSAO_NOISE_SCALE, NoiseScale);
        SSAOShader->SetFloat(SSAO_BIAS, SSAOBias);
        SSAOShader->SetFloat(SSAO_RADIUS, SSAORadius);

        gpu::DrawImmediateQuad({ 0, 0 } , SSAOResult->GetSize());

        // Blur SSAO
        BlurFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        BlurFramebuffer->SetupColorAttachment(0, SSAOBlurred);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));

        SimpleBlurShader->Use();
        SimpleBlurShader->UseTexture(SIMPLE_BLUR_TEXTURE, SSAOResult);
        SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_X, SimpleBlurXOffset);
        SimpleBlurShader->SetInt(SIMPLE_BLUR_OFFSET_Y, SimpleBlurYOffset);

        gpu::DrawImmediateQuad({ 0, 0 } , SSAOResult->GetSize());        
    }

    void ForwardRenderer::LightingPass(const FRenderScene* InSceneToRender, const FRenderView* InRenderView)
    {
        gpu::EnableDepthTest();
        gpu::SetReadOnlyDepthBuffer(true);
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        BindAndClearFramebuffer(LightingPassFramebuffer);
        LightingPassFramebuffer->SetupDrawBuffers();

        DefaultRenderableShader->Use();
        SetupRendererWideUniforms(DefaultRenderableShader, InRenderView);

        RenderStaticGeometry(InSceneToRender, InRenderView);
        if (InSceneToRender->SceneSkybox)
        {
            RenderSkybox(InSceneToRender->SceneSkybox, InRenderView);
        }
    }

    void ForwardRenderer::BindAndClearFramebuffer(gpu::CFramebuffer* InFramebuffer)
    {
        InFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
    }

    void ForwardRenderer::SetupRendererWideUniforms(gpu::CShader* InShader, const FRenderView* InRenderView)
    {
        InShader->SetFloat(AMBIENT_STRENGTH, AmbientStrength);
        InShader->SetInt(NUM_OF_PCF_SAMPLES, NumSamplesPCF);
        InShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());
        InShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        InShader->SetVector(VIEW_POSITION, InRenderView->Camera->Position);
        InShader->SetFloat(PARALLAX_HEIGHT_SCALE, 0.1f);
        InShader->SetVector(VIEWPORT_SIZE, glm::vec2 { InRenderView->Viewport.Width, InRenderView->Viewport.Height });
        InShader->UseTexture(AMBIENT_OCCLUSION, SSAOBlurred);
    }

    void ForwardRenderer::Render(gpu::CShader* Shader, const FRenderable* InRenderable)
    {
        glm::mat4 modelMatrix = InRenderable->CalculateModelMatrix();
        Shader->SetMatrix(MODEL_MATRIX, modelMatrix);
        Shader->SetBool(REVERSE_NORMALS, InRenderable->bReverseNormals);

        InRenderable->Material->SetupShader(Shader);

        InRenderable->VertexArray->Bind();
        InRenderable->VertexArray->Draw();
    }

    inline void ForwardRenderer::RenderSkybox(const FSkybox* InSkybox, const FRenderView* InRenderView)
    {
        gpu::DisableBlending();

        SkyboxShader->Use();

        SkyboxShader->UseTexture(SKYBOX_CUBEMAP, InSkybox->SkyboxCubemap);
        SkyboxShader->SetMatrix(VIEW_MATRIX, InRenderView->Camera->GetViewMatrix());
        SkyboxShader->SetMatrix(PROJECTION_MATRIX, InRenderView->Camera->GetProjectionMatrix());

        misc::CubeVertexArray->Bind();
        misc::CubeVertexArray->Draw();
    }
} // namespace lucid::scene