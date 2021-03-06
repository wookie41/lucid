#include "scene/forward_renderer.hpp"

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

    void ForwardRenderer::Setup(const RenderTarget* InRenderTarget)
    {
        Renderer::Setup(InRenderTarget);
    }

    void ForwardRenderer::Cleanup()
    {
    }

    void ForwardRenderer::Render(const RenderScene* InSceneToRender, const RenderTarget* InRenderTarget)
    {
        const RenderTarget* Target = InRenderTarget ? InRenderTarget : DefaultRenderTarget;

        gpu::EnableDepthTest();
        gpu::SetReadOnlyDepthBuffer(false);

        gpu::SetViewport(Target->Viewport);
        BindAndClearFramebuffer(Target->Framebuffer);
        
        // Depth prepass
        DepthPrePassShader->Use();
        Target->Framebuffer->DisableReadWriteBuffers();
        SetupRendererWideUniforms(DepthPrePassShader, Target);
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        const LinkedListItem<Renderable>* CurrentNode = &InSceneToRender->StaticGeometry.Head;
        while (CurrentNode && CurrentNode->Element)
        {
            Renderable* CurrentRenderable = CurrentNode->Element;
            Render(DepthPrePassShader, CurrentRenderable);
            CurrentNode = CurrentNode->Next;
        }
        
        Target->Framebuffer->SetupDrawBuffers();
        
        // Lighting pass
        gpu::SetReadOnlyDepthBuffer(true);
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        DefaultShader->Use();
        SetupRendererWideUniforms(DefaultShader, Target);

        RenderStaticGeometry(InSceneToRender, Target);
        if (InSceneToRender->SceneSkybox)
        {
            RenderSkybox(InSceneToRender->SceneSkybox, Target);
        }
    }

    void ForwardRenderer::RenderStaticGeometry(const RenderScene* InScene, const RenderTarget* InRenderTarget)
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
            RenderWithoutLights(InScene, InRenderTarget);
            return;
        }

        RenderLightContribution(LightNode->Element, InScene, InRenderTarget);
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
            RenderLightContribution(LightNode->Element, InScene, InRenderTarget);
            LightNode = LightNode->Next;
        }
    }

    void ForwardRenderer::RenderLightContribution(const Light* InLight,
                                                  const RenderScene* InScene,
                                                  const RenderTarget* InRenderTarget)
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
                SetupRendererWideUniforms(customShader, InRenderTarget);
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

    void ForwardRenderer::SetupLight(gpu::Shader* Shader, const Light* InLight)
    {
        Shader->SetVector(LIGHT_COLOR, InLight->Color);
        Shader->SetVector(LIGHT_POSITION, InLight->Position);

        switch (InLight->GetType())
        {
        case LightType::DIRECTIONAL:
        {
            DirectionalLight* light = (DirectionalLight*)InLight;
            Shader->SetVector(LIGHT_DIRECTION, light->Direction);
            Shader->SetInt(LIGHT_TYPE, DIRECTIONAL_LIGHT);
            Shader->SetMatrix(LIGHT_SPACE_MATRIX, light->LightSpaceMatrix);

            if (light->ShadowMap != nullptr)
            {
                Shader->SetBool(LIGHT_CASTS_SHADOWS, true);
                Shader->UseTexture(LIGHT_SHADOW_MAP, light->ShadowMap);
            }
            else
            {
                Shader->SetBool(LIGHT_CASTS_SHADOWS, false);
            }
            break;
        }
        case LightType::POINT:
        {
            PointLight* light = (PointLight*)InLight;
            Shader->SetFloat(LIGHT_CONSTANT, light->Constant);
            Shader->SetFloat(LIGHT_LINEAR, light->Linear);
            Shader->SetFloat(LIGHT_QUADRATIC, light->Quadratic);
            Shader->SetFloat(LIGHT_FAR_PLANE, light->FarPlane);
            Shader->SetInt(LIGHT_TYPE, POINT_LIGHT);

            if (light->ShadowMap != nullptr)
            {
                Shader->SetBool(LIGHT_CASTS_SHADOWS, true);
                Shader->UseTexture(LIGHT_SHADOW_CUBE, light->ShadowMap);
            }
            else
            {
                Shader->SetBool(LIGHT_CASTS_SHADOWS, false);
            }

            break;
        }
        case LightType::SPOT:
        {
            SpotLight* light = (SpotLight*)InLight;
            Shader->SetVector(LIGHT_DIRECTION, light->Direction);
            Shader->SetFloat(LIGHT_CONSTANT, light->Constant);
            Shader->SetFloat(LIGHT_LINEAR, light->Linear);
            Shader->SetFloat(LIGHT_QUADRATIC, light->Quadratic);
            Shader->SetFloat(LIGHT_INNER_CUT_OFF, glm::cos(light->InnerCutOffRad));
            Shader->SetFloat(LIGHT_OUTER_CUT_OFF, glm::cos(light->OuterCutOffRad));
            Shader->SetInt(LIGHT_TYPE, SPOT_LIGHT);
            Shader->SetMatrix(LIGHT_SPACE_MATRIX, light->LightSpaceMatrix);

            if (light->ShadowMap != nullptr)
            {
                Shader->SetBool(LIGHT_CASTS_SHADOWS, true);
                Shader->UseTexture(LIGHT_SHADOW_MAP, light->ShadowMap);
            }
            else
            {
                Shader->SetBool(LIGHT_CASTS_SHADOWS, false);
            }
            break;
        }
        }
    }

    void ForwardRenderer::RenderWithoutLights(const RenderScene* InScene,
                                              const RenderTarget* InRenderTarget)
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
                SetupRendererWideUniforms(CustomShader, InRenderTarget);
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

    void ForwardRenderer::BindAndClearFramebuffer(gpu::Framebuffer* Framebuffer)
    {
        Framebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::ClearableBuffers)(gpu::ClearableBuffers::COLOR | gpu::ClearableBuffers::DEPTH));
    }

    void ForwardRenderer::SetupRendererWideUniforms(gpu::Shader* Shader, const RenderTarget* Target)
    {
        Shader->SetFloat(AMBIENT_STRENGTH, AmbientStrength);
        Shader->SetInt(NUM_OF_PCF_SAMPLES, NumSamplesPCF);
        Shader->SetMatrix(PROJECTION_MATRIX, Target->Camera->GetProjectionMatrix());
        Shader->SetMatrix(VIEW_MATRIX, Target->Camera->GetViewMatrix());
        Shader->SetVector(VIEW_POSITION, Target->Camera->Position);
        Shader->SetFloat(PARALLAX_HEIGHT_SCALE, 0.1f);
    }

    void ForwardRenderer::Render(gpu::Shader* Shader, Renderable* const ToRender)
    {
        glm::mat4 modelMatrix = ToRender->CalculateModelMatrix();
        Shader->SetMatrix(MODEL_MATRIX, modelMatrix);
        Shader->SetBool(REVERSE_NORMALS, ToRender->bReverseNormals);

        ToRender->Material->SetupShader(Shader);

        ToRender->VertexArray->Bind();
        ToRender->VertexArray->Draw();
    }

    inline void ForwardRenderer::RenderSkybox(const Skybox* InSkybox, const RenderTarget* InRenderTarget)
    {
        gpu::DisableBlending();

        SkyboxShader->Use();

        SkyboxShader->UseTexture(SKYBOX_CUBEMAP, InSkybox->SkyboxCubemap);
        SkyboxShader->SetMatrix(VIEW_MATRIX, InRenderTarget->Camera->GetViewMatrix());
        SkyboxShader->SetMatrix(PROJECTION_MATRIX, InRenderTarget->Camera->GetProjectionMatrix());

        misc::CubeVertexArray->Bind();
        misc::CubeVertexArray->Draw();
    }
} // namespace lucid::scene