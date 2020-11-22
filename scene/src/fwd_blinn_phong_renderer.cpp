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

    static const uint32_t NO_LIGHT = 0;
    static const uint32_t DIRECTIONAL_LIGHT = 1;
    static const uint32_t POINT_LIGHT = 2;
    static const uint32_t SPOT_LIGHT = 3;

    static const String LIGHT_TO_USE("uLightToUse");

    static const String DIRECTIONAL_LIGHT_DIRECTION("uDirectionalLight.Direction");
    static const String DIRECTIONAL_LIGHT_COLOR("uDirectionalLight.Color");

    static const String POINT_LIGHT_POSITION("uPointLight.Position");
    static const String POINT_LIGHT_COLOR("uPointLight.Color");
    static const String POINT_LIGHT_CONSTANT("uPointLight.Constant");
    static const String POINT_LIGHT_LINEAR("uPointLight.Linear");
    static const String POINT_LIGHT_QUADRATIC("uPointLight.Quadratic");

    static const String SPOT_LIGHT_POSITION("uSpotLight.Position");
    static const String SPOT_LIGHT_DIRECTION("uSpotLight.Direction");
    static const String SPOT_LIGHT_COLOR("uSpotLight.Color");
    static const String SPOT_LIGHT_CONSTANT("uSpotLight.Constant");
    static const String SPOT_LIGHT_LINEAR("uSpotLight.Linear");
    static const String SPOT_LIGHT_QUADRATIC("uSpotLight.Quadratic");
    static const String SPOT_LIGHT_INNER_CUT_OFF("uSpotLight.InnerCutOffCos");
    static const String SPOT_LIGHT_OUTER_CUT_OFF("uSpotLight.OuterCutOffCos");

    // Shader-wide uniforms
    static const String AMBIENT_STRENGTH("uAmbientStrength");

    static const String VIEW_POSITION("uViewPos");

    static const String MODEL_MATRIX("uModel");
    static const String VIEW_MATRIX("uView");
    static const String PROJECTION_MATRIX("uProjection");

    static const String SKYBOX_CUBEMAP("uSkybox");

    ForwardBlinnPhongRenderer::ForwardBlinnPhongRenderer(const uint32_t& MaxNumOfDirectionalLights,
                                                         gpu::Shader* DefaultShader,
                                                         gpu::Shader* SkyboxShader)
    : Renderer(DefaultShader), maxNumOfDirectionalLights(MaxNumOfDirectionalLights), skyboxShader(SkyboxShader)
    {
        SkyboxShader->Use();
        skyboxCubemapUnifomId =  skyboxShader->GetTextureId(SKYBOX_CUBEMAP);
        skyboxViewMatrixUniformId =  skyboxShader->GetIdForUniform(VIEW_MATRIX);
        skyboxProjectionMatrixUniformId = skyboxShader->GetIdForUniform(PROJECTION_MATRIX);
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
        gpu::SetBlendFunctionSeparate(  gpu::BlendFunction::ONE, gpu::BlendFunction::ZERO, 
                                        gpu::BlendFunction::ONE, gpu::BlendFunction::ZERO );

        auto lightNode = &SceneToRender->Lights.Head;
        if (!lightNode->Element)
        {
            // no lights in the scene, render the geometry only with ambient contribution
            RenderWithoutLights(SceneToRender, Target);
            return;
        }

        RenderLightContribution(lightNode->Element, SceneToRender, Target);

        if (!lightNode->Next || lightNode->Next->Element)
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
            else if (usedShader != defaultShader)
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
        switch (InLight->GetType())
        {
        case LightType::DIRECTIONAL:
        {
            DirectionalLight* light = (DirectionalLight*)InLight;
            Shader->SetInt(Shader->GetIdForUniform(LIGHT_TO_USE), DIRECTIONAL_LIGHT);
            Shader->SetVector(Shader->GetIdForUniform(DIRECTIONAL_LIGHT_DIRECTION), light->Direction);
            Shader->SetVector(Shader->GetIdForUniform(DIRECTIONAL_LIGHT_COLOR), light->Color);
        }
        break;
        case LightType::POINT:
        {
            PointLight* light = (PointLight*)InLight;
            Shader->SetInt(Shader->GetIdForUniform(LIGHT_TO_USE), POINT_LIGHT);
            Shader->SetVector(Shader->GetIdForUniform(POINT_LIGHT_POSITION), light->Position);
            Shader->SetVector(Shader->GetIdForUniform(POINT_LIGHT_COLOR), light->Color);
            Shader->SetFloat(Shader->GetIdForUniform(POINT_LIGHT_CONSTANT), light->Constant);
            Shader->SetFloat(Shader->GetIdForUniform(POINT_LIGHT_LINEAR), light->Linear);
            Shader->SetFloat(Shader->GetIdForUniform(POINT_LIGHT_QUADRATIC), light->Quadratic);
        }
        break;
        case LightType::SPOT:
        {
            SpotLight* light = (SpotLight*)InLight;
            Shader->SetInt(Shader->GetIdForUniform(LIGHT_TO_USE), SPOT_LIGHT);
            Shader->SetVector(Shader->GetIdForUniform(SPOT_LIGHT_POSITION), light->Position);
            Shader->SetVector(Shader->GetIdForUniform(SPOT_LIGHT_DIRECTION), light->Direction);
            Shader->SetVector(Shader->GetIdForUniform(SPOT_LIGHT_COLOR), light->Color);
            Shader->SetFloat(Shader->GetIdForUniform(SPOT_LIGHT_CONSTANT), light->Constant);
            Shader->SetFloat(Shader->GetIdForUniform(SPOT_LIGHT_LINEAR), light->Linear);
            Shader->SetFloat(Shader->GetIdForUniform(SPOT_LIGHT_QUADRATIC), light->Quadratic);
            Shader->SetFloat(Shader->GetIdForUniform(SPOT_LIGHT_INNER_CUT_OFF), glm::cos(light->InnerCutOffRad));
            Shader->SetFloat(Shader->GetIdForUniform(SPOT_LIGHT_OUTER_CUT_OFF), glm::cos(light->OuterCutOffRad));
        }
        break;
        }
    }

    void ForwardBlinnPhongRenderer::RenderWithoutLights(RenderScene* const SceneToRender, RenderTarget* const Target)
    {
        auto usedShader = defaultShader;
        auto currentNode = &SceneToRender->StaticGeometry.Head;

        defaultShader->SetInt(defaultShader->GetIdForUniform(LIGHT_TO_USE), NO_LIGHT);

        while (currentNode && currentNode->Element)
        {
            auto renderable = currentNode->Element;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provided uniforms

            auto customShader = renderable->Material->GetCustomShader();
            if (customShader)
            {
                customShader->Use();
                customShader->SetInt(defaultShader->GetIdForUniform(LIGHT_TO_USE), NO_LIGHT);
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
        Shader->SetFloat(Shader->GetIdForUniform(AMBIENT_STRENGTH), ambientStrength);
        Shader->SetMatrix(Shader->GetIdForUniform(PROJECTION_MATRIX), Target->Camera->GetProjectionMatrix());
        Shader->SetMatrix(Shader->GetIdForUniform(VIEW_MATRIX), Target->Camera->GetViewMatrix());
        Shader->SetVector(Shader->GetIdForUniform(VIEW_POSITION), Target->Camera->Position);
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
        Shader->SetMatrix(Shader->GetIdForUniform(MODEL_MATRIX), modelMatrix);

        ToRender->Material->SetupShader(Shader);

        ToRender->VertexArray->Bind();
        ToRender->VertexArray->Draw();
    }

    inline void ForwardBlinnPhongRenderer::RenderSkybox(Skybox * const SkyboxToRender, const RenderTarget* RenderTarget)
    {
        gpu::DisableBlending();

        skyboxShader->Use();

        skyboxShader->UseTexture(skyboxCubemapUnifomId, SkyboxToRender->SkyboxCubemap);
        skyboxShader->SetMatrix(skyboxViewMatrixUniformId, RenderTarget->Camera->GetViewMatrix());
        skyboxShader->SetMatrix(skyboxProjectionMatrixUniformId, RenderTarget->Camera->GetProjectionMatrix());

        misc::CubeVertexArray->Bind();
        misc::CubeVertexArray->Draw();
    }

} // namespace lucid::scene