#include "scene/forward_renderer.hpp"

#include <GL/glew.h>



#include "common/log.hpp"
#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"
#include "scene/lights.hpp"

#include "scene/renderable.hpp"
#include "scene/camera.hpp"
#include "scene/blinn_phong_material.hpp"

#include "devices/gpu/gpu.hpp"

#include "devices/gpu/cubemap.hpp"
#include "devices/gpu/viewport.hpp"
#include "misc/basic_shapes.hpp"

namespace lucid::scene
{
    // Shader light-related uniforms
    static const u32 NO_LIGHT = 0;
    static const u32 DIRECTIONAL_LIGHT = 1;
    static const u32 POINT_LIGHT = 2;
    static const u32 SPOT_LIGHT = 3;

    static const String LIGHT_TYPE("uLight.Type");

    static const String LIGHT_POSITION("uLight.Position");
    static const String LIGHT_DIRECTION("uLight.Direction");
    static const String LIGHT_COLOR("uLight.Color");
    static const String LIGHT_CONSTANT("uLight.Constant");
    static const String LIGHT_LINEAR("uLight.Linear");
    static const String LIGHT_QUADRATIC("uLight.Quadratic");
    static const String LIGHT_INNER_CUT_OFF("uLight.InnerCutOffCos");
    static const String LIGHT_OUTER_CUT_OFF("uLight.OuterCutOffCos");
    static const String LIGHT_SHADOW_MAP("uLight.ShadowMap");
    static const String LIGHT_SPACE_MATRIX("uLight.LightSpaceMatrix");
    static const String LIGHT_CASTS_SHADOWS("uLight.CastsShadows");
    static const String LIGHT_FAR_PLANE("uLight.FarPlane");
    static const String LIGHT_SHADOW_CUBE("uShadowCube");

    // Shader-wide uniforms
    static const String AMBIENT_STRENGTH("uAmbientStrength");
    static const String NUM_OF_PCF_SAMPLES("uNumSamplesPCF");

    static const String VIEW_POSITION("uViewPos");

    static const String MODEL_MATRIX("uModel");
    static const String REVERSE_NORMALS("uReverseNormals");
    static const String VIEW_MATRIX("uView");
    static const String PROJECTION_MATRIX("uProjection");

    static const String SKYBOX_CUBEMAP("uSkybox");

    static const String PARALLAX_HEIGHT_SCALE("uParallaxHeightScale");

    ForwardRenderer::ForwardRenderer(const u32& InMaxNumOfDirectionalLights,
                                     gpu::Shader* InDefaultShader,
                                     gpu::Shader* InDepthPrepassShader,
                                     gpu::Shader* InSkyboxShader)
    : Renderer(InDefaultShader), MaxNumOfDirectionalLights(InMaxNumOfDirectionalLights), SkyboxShader(InSkyboxShader),
      DepthPrePassShader(InDepthPrepassShader)
    {
    }

    void ForwardRenderer::Setup()
    {
        // Create the framebuffers
        PrepassFramebuffer = gpu::CreateFramebuffer();
        LightingPassFramebuffer = gpu::CreateFramebuffer();

        // Create a common depth-stencil attachment for both framebuffers
        DepthStencilRenderBuffer = gpu::CreateRenderbuffer(gpu::RenderbufferFormat::DEPTH24_STENCIL8, FramebufferSize);

        // Create render targets in which we'll store some additional information during the depth prepass
        CurrentFrameVSNormalMap = gpu::CreateEmpty2DTexture(FramebufferSize.x, FramebufferSize.y, gpu::TextureDataType::FLOAT, gpu::TextureFormat::RGB16F, 0);
        CurrentFrameVSPositionMap = gpu::CreateEmpty2DTexture(FramebufferSize.x, FramebufferSize.y, gpu::TextureDataType::FLOAT, gpu::TextureFormat::RGB16F, 0);
        
        // Setup the prepass framebuffer
        PrepassFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);

        // CurrentFrameVSNormalMap->Bind();
        // PrepassFramebuffer->SetupColorAttachment(0, CurrentFrameVSNormalMap);

        // CurrentFrameVSPositionMap->Bind();
        // PrepassFramebuffer->SetupColorAttachment(1, CurrentFrameVSPositionMap);

        DepthStencilRenderBuffer->Bind();
        PrepassFramebuffer->SetupDepthStencilAttachment(DepthStencilRenderBuffer);

        if (!PrepassFramebuffer->IsComplete())
        {
            LUCID_LOG(LogLevel::ERR, LUCID_TEXT("Failed to setup the prepass framebuffer"));
            return;
        }

        // Create color attachment for the lighting pass framebuffer
        LightingPassColorBuffer = gpu::CreateEmpty2DTexture(FramebufferSize.x, FramebufferSize.y, gpu::TextureDataType::FLOAT, gpu::TextureFormat::RGBA, 0);

        // Setup the lighting pass framebuffer
        LightingPassFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);

        LightingPassColorBuffer->Bind();
        LightingPassFramebuffer->SetupColorAttachment(0, LightingPassColorBuffer);

        DepthStencilRenderBuffer->Bind();
        LightingPassFramebuffer->SetupDepthStencilAttachment(DepthStencilRenderBuffer);

        if (!LightingPassFramebuffer->IsComplete())
        {
            LUCID_LOG(LogLevel::ERR, LUCID_TEXT("Failed to setup the lighting framebuffer"));
            return;
        }
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

    void ForwardRenderer::Render(const RenderScene* InSceneToRender, const RenderSource* InRenderSource)
    {
        gpu::EnableDepthTest();
        gpu::DisableSRGBFramebuffer();
        gpu::SetViewport(InRenderSource->Viewport);
        
        // Depth prepass
        gpu::SetReadOnlyDepthBuffer(false);
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        BindAndClearFramebuffer(PrepassFramebuffer);

        DepthPrePassShader->Use();
        SetupRendererWideUniforms(DepthPrePassShader, InRenderSource);

        const LinkedListItem<Renderable>* CurrentNode = &InSceneToRender->StaticGeometry.Head;
        while (CurrentNode && CurrentNode->Element)
        {
            Renderable* CurrentRenderable = CurrentNode->Element;
            Render(DepthPrePassShader, CurrentRenderable);
            CurrentNode = CurrentNode->Next;
        }
        
        // Lighting pass
        gpu::SetReadOnlyDepthBuffer(true);
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        BindAndClearFramebuffer(LightingPassFramebuffer);

        DefaultShader->Use();
        SetupRendererWideUniforms(DefaultShader, InRenderSource);

        RenderStaticGeometry(InSceneToRender, InRenderSource);
        if (InSceneToRender->SceneSkybox)
        {
            RenderSkybox(InSceneToRender->SceneSkybox, InRenderSource);
        }
    }

    void ForwardRenderer::RenderStaticGeometry(const RenderScene* InScene, const RenderSource* InRenderSource)
    {
        gpu::SetCullMode(gpu::CullMode::BACK);

        // Render with lights contribution, ie. update the lighting uniforms,
        // as the underlying shader will use them when rendering the geometry

        gpu::EnableBlending();
        gpu::SetBlendFunctionSeparate(gpu::BlendFunction::ONE, gpu::BlendFunction::ZERO, gpu::BlendFunction::ONE,
                                      gpu::BlendFunction::ZERO);

        const LinkedListItem<Light>* LightNode = &InScene->Lights.Head;
        if (!LightNode->Element)
        {
            // no lights in the scene, render the geometry only with ambient contribution
            RenderWithoutLights(InScene, InRenderSource);
            return;
        }

        RenderLightContribution(LightNode->Element, InScene, InRenderSource);
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
            RenderLightContribution(LightNode->Element, InScene, InRenderSource);
            LightNode = LightNode->Next;
        }
    }

    void ForwardRenderer::RenderLightContribution(const Light* InLight,
                                                  const RenderScene* InScene,
                                                  const RenderSource* InRenderSource)
    {
        const LinkedListItem<Renderable>* CurrentNode = &InScene->StaticGeometry.Head;
        gpu::Shader* LastUsedShader = DefaultShader;
        while (CurrentNode && CurrentNode->Element)
        {
            Renderable* CurrentRenderable = CurrentNode->Element;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms
            auto customShader = CurrentRenderable->Material->GetCustomShader();
            if (customShader)
            {
                customShader->Use();
                SetupRendererWideUniforms(customShader, InRenderSource);
                LastUsedShader = customShader;
            }
            else
            {
                DefaultShader->Use();
                LastUsedShader = DefaultShader;
            }

            SetupLight(LastUsedShader, InLight);
            Render(LastUsedShader, CurrentRenderable);

            CurrentNode = CurrentNode->Next;
        }
    }

    void ForwardRenderer::SetupLight(gpu::Shader* InShader, const Light* InLight)
    {
        InShader->SetVector(LIGHT_COLOR, InLight->Color);
        InShader->SetVector(LIGHT_POSITION, InLight->Position);

        switch (InLight->GetType())
        {
        case LightType::DIRECTIONAL:
        {
            DirectionalLight* light = (DirectionalLight*)InLight;
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
        case LightType::POINT:
        {
            PointLight* light = (PointLight*)InLight;
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
        case LightType::SPOT:
        {
            SpotLight* light = (SpotLight*)InLight;
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

    void ForwardRenderer::RenderWithoutLights(const RenderScene* InScene,
                                              const RenderSource* InRenderSource)
    {
        const LinkedListItem<Renderable>* CurrentNode = &InScene->StaticGeometry.Head;
        gpu::Shader* LastUserShader = DefaultShader;
        
        LastUserShader->SetInt(LIGHT_TYPE, NO_LIGHT);

        while (CurrentNode && CurrentNode->Element)
        {
            Renderable* CurrentRenderable = CurrentNode->Element;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms
            gpu::Shader* CustomShader = CurrentRenderable->Material->GetCustomShader();
            if (CustomShader)
            {
                CustomShader->Use();
                CustomShader->SetInt(LIGHT_TYPE, NO_LIGHT);
                SetupRendererWideUniforms(CustomShader, InRenderSource);
                LastUserShader = CustomShader;
            }
            else if (LastUserShader != DefaultShader)
            {
                DefaultShader->Use();
                LastUserShader = DefaultShader;
            }

            Render(LastUserShader, CurrentRenderable);
            CurrentNode = CurrentNode->Next;
        }
    }

    void ForwardRenderer::BindAndClearFramebuffer(gpu::Framebuffer* InFramebuffer)
    {
        InFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
    }

    void ForwardRenderer::SetupRendererWideUniforms(gpu::Shader* InShader, const RenderSource* InRenderSource)
    {
        InShader->SetFloat(AMBIENT_STRENGTH, AmbientStrength);
        InShader->SetInt(NUM_OF_PCF_SAMPLES, NumSamplesPCF);
        InShader->SetMatrix(PROJECTION_MATRIX, InRenderSource->Camera->GetProjectionMatrix());
        InShader->SetMatrix(VIEW_MATRIX, InRenderSource->Camera->GetViewMatrix());
        InShader->SetVector(VIEW_POSITION, InRenderSource->Camera->Position);
        InShader->SetFloat(PARALLAX_HEIGHT_SCALE, 0.1f);
    }

    void ForwardRenderer::Render(gpu::Shader* Shader, const Renderable* InRenderable)
    {
        glm::mat4 modelMatrix = InRenderable->CalculateModelMatrix();
        Shader->SetMatrix(MODEL_MATRIX, modelMatrix);
        Shader->SetBool(REVERSE_NORMALS, InRenderable->bReverseNormals);

        InRenderable->Material->SetupShader(Shader);

        InRenderable->VertexArray->Bind();
        InRenderable->VertexArray->Draw();
    }

    inline void ForwardRenderer::RenderSkybox(const Skybox* InSkybox, const RenderSource* InRenderSource)
    {
        gpu::DisableBlending();

        SkyboxShader->Use();

        SkyboxShader->UseTexture(SKYBOX_CUBEMAP, InSkybox->SkyboxCubemap);
        SkyboxShader->SetMatrix(VIEW_MATRIX, InRenderSource->Camera->GetViewMatrix());
        SkyboxShader->SetMatrix(PROJECTION_MATRIX, InRenderSource->Camera->GetProjectionMatrix());

        misc::CubeVertexArray->Bind();
        misc::CubeVertexArray->Draw();
    }
} // namespace lucid::scene