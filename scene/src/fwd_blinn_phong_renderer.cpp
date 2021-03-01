#include "scene/fwd_blinn_phong_renderer.hpp"

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"
#include "scene/lights.hpp"

#include "scene/renderable.hpp"
#include "scene/camera.hpp"
#include "scene/blinn_phong_material.hpp"

#include "resources/texture.hpp"
#include "resources/mesh.hpp"

#include "common/log.hpp"
#include "devices/gpu/gpu.hpp"

#include "devices/gpu/cubemap.hpp"
#include "misc/basic_shapes.hpp"

#include <stdio.h>
#include "GL/glew.h"

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

    ForwardBlinnPhongRenderer::ForwardBlinnPhongRenderer(const u32& MaxNumOfDirectionalLights,
                                                         gpu::Shader* DefaultShader,
                                                         gpu::Shader* SkyboxShader)
    : Renderer(DefaultShader), maxNumOfDirectionalLights(MaxNumOfDirectionalLights), skyboxShader(SkyboxShader)
    {
        SkyboxShader->Use();
    }

    void ForwardBlinnPhongRenderer::Render(RenderScene* const SceneToRender, RenderTarget* const Target)
    {
        SetupFramebuffer(Target->Framebuffer);
        gpu::SetViewport(Target->Viewport);

        defaultShader->Use();

        SetupRendererWideUniforms(defaultShader, Target);
        RenderStaticGeometry(SceneToRender, Target);

        if (SceneToRender->SceneSkybox)
        {
            RenderSkybox(SceneToRender->SceneSkybox, Target);
        }
    }

    void ForwardBlinnPhongRenderer::RenderStaticGeometry(RenderScene* const SceneToRender, RenderTarget* const Target)
    {
        gpu::EnableDepthTest();
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        gpu::EnableBlending();
        gpu::SetBlendFunctionSeparate(gpu::BlendFunction::ONE, gpu::BlendFunction::ZERO, gpu::BlendFunction::ONE,
                                      gpu::BlendFunction::ZERO);

        gpu::SetCullMode(gpu::CullMode::BACK);

        auto lightNode = &SceneToRender->Lights.Head;
        if (!lightNode->Element)
        {
            // no lights in the scene, render the geometry only with ambient contribution
            RenderWithoutLights(SceneToRender, Target);
            return;
        }

        RenderLightContribution(lightNode->Element, SceneToRender, Target);

        if (!lightNode->Next || !lightNode->Next->Element)
        {
            // No more lights
            return;
        }

        // Change the blending mode so we render the rest of the lights additively
        gpu::SetBlendFunction(gpu::BlendFunction::ONE, gpu::BlendFunction::ONE);

        lightNode = lightNode->Next;

        while (lightNode && lightNode->Element)
        {
            RenderLightContribution(lightNode->Element, SceneToRender, Target);
            lightNode = lightNode->Next;
        }
    }

    void ForwardBlinnPhongRenderer::RenderLightContribution(Light* const InLight,
                                                            RenderScene* const SceneToRender,
                                                            RenderTarget* const Target)
    {
        auto usedShader = defaultShader;
        auto currentNode = &SceneToRender->StaticGeometry.Head;

        while (currentNode && currentNode->Element)
        {
            auto renderable = currentNode->Element;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms

            auto customShader = renderable->Material->GetCustomShader();
            if (customShader)
            {
                customShader->Use();
                SetupRendererWideUniforms(customShader, Target);
                usedShader = customShader;
            }
            else
            {
                defaultShader->Use();
                usedShader = defaultShader;
            }

            SetupLight(usedShader, InLight);
            Render(usedShader, renderable);

            currentNode = currentNode->Next;
        }
    }

    void ForwardBlinnPhongRenderer::SetupLight(gpu::Shader* Shader, Light* const InLight)
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

    void ForwardBlinnPhongRenderer::RenderWithoutLights(RenderScene* const SceneToRender, RenderTarget* const Target)
    {
        auto usedShader = defaultShader;
        auto currentNode = &SceneToRender->StaticGeometry.Head;

        defaultShader->SetInt(LIGHT_TYPE, NO_LIGHT);

        while (currentNode && currentNode->Element)
        {
            auto renderable = currentNode->Element;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms

            auto customShader = renderable->Material->GetCustomShader();
            if (customShader)
            {
                customShader->Use();
                customShader->SetInt(LIGHT_TYPE, NO_LIGHT);
                SetupRendererWideUniforms(customShader, Target);
                usedShader = customShader;
            }
            else if (usedShader != defaultShader)
            {
                defaultShader->Use();
                usedShader = defaultShader;
            }

            Render(usedShader, renderable);

            currentNode = currentNode->Next;
        }
    }

    void ForwardBlinnPhongRenderer::SetupFramebuffer(gpu::Framebuffer* Framebuffer)
    {
        if (Framebuffer)
        {
            Framebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        }
        else
        {
            gpu::BindDefaultFramebuffer(gpu::FramebufferBindMode::READ_WRITE);
        }
    }

    void ForwardBlinnPhongRenderer::SetupRendererWideUniforms(gpu::Shader* Shader, const RenderTarget* Target)
    {
        Shader->SetFloat(AMBIENT_STRENGTH, AmbientStrength);
        Shader->SetInt(NUM_OF_PCF_SAMPLES, NumSamplesPCF);
        Shader->SetMatrix(PROJECTION_MATRIX, Target->Camera->GetProjectionMatrix());
        Shader->SetMatrix(VIEW_MATRIX, Target->Camera->GetViewMatrix());
        Shader->SetVector(VIEW_POSITION, Target->Camera->Position);
        Shader->SetFloat(PARALLAX_HEIGHT_SCALE, 0.1f);
    }

    Renderable* CreateBlinnPhongRenderable(DString MeshName, resources::MeshResource* Mesh, gpu::Shader* CustomShader)
    {
        auto fallbackTexture = resources::TexturesHolder.GetDefaultResource()->TextureHandle;

        //@TODO Material caching?
        BlinnPhongMapsMaterial* meshMaterial = new BlinnPhongMapsMaterial(CustomShader);
        meshMaterial->Shininess = 32;

        if (Mesh->DiffuseMap == nullptr)
        {
            LUCID_LOG(LogLevel::INFO, "Mesh is missing a diffuse map");
            meshMaterial->DiffuseMap = fallbackTexture;
        }
        else
        {
            meshMaterial->DiffuseMap = Mesh->DiffuseMap->TextureHandle;
        }

        if (Mesh->SpecularMap == nullptr)
        {
            LUCID_LOG(LogLevel::INFO, "Mesh is missing a specular map");
            meshMaterial->SpecularMap = fallbackTexture;
        }
        else
        {
            meshMaterial->SpecularMap = Mesh->SpecularMap->TextureHandle;
        }

        if (Mesh->NormalMap == nullptr)
        {
            LUCID_LOG(LogLevel::INFO, "Mesh is missing a normal map");
            meshMaterial->NormalMap = fallbackTexture;
        }
        else
        {
            meshMaterial->NormalMap = Mesh->NormalMap->TextureHandle;
        }

        Renderable* meshRenderable = new Renderable{ MeshName };
        meshRenderable->Material = meshMaterial;
        meshRenderable->Type = RenderableType::STATIC;
        meshRenderable->VertexArray = Mesh->VAO;

        return meshRenderable;
    }

    void ForwardBlinnPhongRenderer::Render(gpu::Shader* Shader, Renderable* const ToRender)
    {
        glm::mat4 modelMatrix = ToRender->CalculateModelMatrix();
        Shader->SetMatrix(MODEL_MATRIX, modelMatrix);
        Shader->SetBool(REVERSE_NORMALS, ToRender->bReverseNormals);

        ToRender->Material->SetupShader(Shader);

        ToRender->VertexArray->Bind();
        ToRender->VertexArray->Draw();
    }

    inline void ForwardBlinnPhongRenderer::RenderSkybox(Skybox* const SkyboxToRender, const RenderTarget* RenderTarget)
    {
        gpu::DisableBlending();

        skyboxShader->Use();

        skyboxShader->UseTexture(SKYBOX_CUBEMAP, SkyboxToRender->SkyboxCubemap);
        skyboxShader->SetMatrix(VIEW_MATRIX, RenderTarget->Camera->GetViewMatrix());
        skyboxShader->SetMatrix(PROJECTION_MATRIX, RenderTarget->Camera->GetProjectionMatrix());

        misc::CubeVertexArray->Bind();
        misc::CubeVertexArray->Draw();
    }

} // namespace lucid::scene