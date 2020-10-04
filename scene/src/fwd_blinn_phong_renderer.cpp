#include "scene/fwd_blinn_phong_renderer.hpp"

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"

#include "scene/renderable.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"

#include "common/log.hpp"

namespace lucid::scene
{
    // Shader-wide uniforms
    static const String AMBIENT_STRENGTH("uAmbientStrength");

    static const String LIGHT_POSITION("uLightPos");
    static const String VIEW_POSITION("uViewPos");

    static const String MODEL_MATRIX("uModel");
    static const String VIEW_MATRIX("uView");
    static const String PROJECTION_MATRIX("uProjection");

    static const String NUM_OF_DIRECTIONAL_LIGHTS("uNumOfDirectionalLight");
    static const String DIRECTIONAL_LIGHT_DIRECTION("uDirectionalLights[%d].Direction");
    static const String DIRECTIONAL_LIGHT_COLOR("uDirectionalLights[%d].Color");

    void ForwardBlinnPhongRenderer::Render(const RenderScene* SceneToRender, const RenderTarget* Target)
    {
        SetupFramebuffer(Target->Framebuffer);
        gpu::SetViewport(Target->Viewport);

        defaultShader->Use();
        SetupRendererWideUniforms(defaultShader, Target);
        SetupLights(defaultShader, SceneToRender);

        auto usedShader = defaultShader;
        auto currentNode = &SceneToRender->Renderables.Head;

        // TODO sort by shader in the material to avoid costly program switches

        while (currentNode && currentNode->Element)
        {
            auto renderable = currentNode->Element;
            LUCID_LOG(LogLevel::INFO, "Rendering '%s'", renderable->Name.CString);

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provied uniforms

            auto customShader = renderable->Material->GetCustomShader();
            if (customShader)
            {
                usedShader = customShader;
                customShader->Use();
                SetupRendererWideUniforms(customShader, Target);
                SetupLights(defaultShader, SceneToRender);
            }

            // calculate and set the model matrix

            glm::mat4 modelMatrix{ 1 };
            modelMatrix = glm::translate(modelMatrix, renderable->Transform.Translation);
            modelMatrix =
            glm::rotate(modelMatrix, renderable->Transform.Rotation.w,
                        { renderable->Transform.Rotation.x, renderable->Transform.Rotation.y,
                          renderable->Transform.Rotation.z });
            modelMatrix = glm::scale(modelMatrix, renderable->Transform.Scale); // cache ?

            usedShader->SetMatrix(MODEL_MATRIX, modelMatrix);

            // setup the sahder's uniform using material's properties

            renderable->Material->SetupShader(usedShader);

            // bind the renderable's vertex array and draw it

            renderable->VertexArray->Bind();
            renderable->VertexArray->Draw();

            // if the renderable used a custom material, then restore the default one

            if (usedShader != defaultShader)
            {
                usedShader = defaultShader;
                defaultShader->Use();
            }

            currentNode = currentNode->Next;
        }

        if (Target->Framebuffer)
        {
            Target->Framebuffer->Unbind();
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
        Shader->SetFloat(AMBIENT_STRENGTH, ambientStrength);
        Shader->SetMatrix(PROJECTION_MATRIX, Target->Camera->GetProjectionMatrix());
        Shader->SetMatrix(VIEW_MATRIX, Target->Camera->GetViewMatrix());
        Shader->SetVector(VIEW_POSITION, Target->Camera->Position);
    }

    void ForwardBlinnPhongRenderer::SetupLights(gpu::Shader* Shader, const RenderScene* Scene)
    {
        static char uniformNameBuffer[128];
        auto lightNode = &Scene->DirectionalLights.Head;

        uint32_t directionalLighsCount = 0;
        for (uint32_t ligthIdx = 0; ligthIdx < maxNumOfDirectionalLights; ++ligthIdx, ++directionalLighsCount)
        {
            if (lightNode == nullptr || lightNode->Element == nullptr)
            {
                // no more directional lights in the scene
                break;
            }

            sprintf(uniformNameBuffer, DIRECTIONAL_LIGHT_DIRECTION, ligthIdx);
            Shader->SetVector(uniformNameBuffer, lightNode->Element->Direction);

            sprintf(uniformNameBuffer, DIRECTIONAL_LIGHT_COLOR, ligthIdx);
            Shader->SetVector(uniformNameBuffer, lightNode->Element->Color);

            lightNode = lightNode->Next;
        }
        
        Shader->SetInt(NUM_OF_DIRECTIONAL_LIGHTS, directionalLighsCount);
    }
} // namespace lucid::scene