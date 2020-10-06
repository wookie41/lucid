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

    ForwardBlinnPhongRenderer::ForwardBlinnPhongRenderer(const uint32_t& MaxNumOfDirectionalLights,
                                                         gpu::Shader* DefaultShader)
    : Renderer(DefaultShader), maxNumOfDirectionalLights(MaxNumOfDirectionalLights)
    {
        modelMatrixUniformId = DefaultShader->GetIdForUniform(MODEL_MATRIX);
        ambientStrengthUniformId = DefaultShader->GetIdForUniform(AMBIENT_STRENGTH);
        projectionMatrixUniformId = DefaultShader->GetIdForUniform(PROJECTION_MATRIX);
        viewMatrixUniformId = DefaultShader->GetIdForUniform(VIEW_MATRIX);
        viewPositionUniformId = DefaultShader->GetIdForUniform(VIEW_POSITION);
    }

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

            uint32_t modelMatrixUniformId = this->modelMatrixUniformId;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provied uniforms

            auto customShader = renderable->Material->GetCustomShader();
            if (customShader)
            {
                usedShader = customShader;
                customShader->Use();
                SetupRendererWideUniforms(customShader, Target);
                SetupLights(customShader, SceneToRender);
                modelMatrixUniformId = customShader->GetIdForUniform(MODEL_MATRIX);
            }

            // calculate and set the model matrix

            glm::mat4 modelMatrix{ 1 };
            modelMatrix = glm::translate(modelMatrix, renderable->Transform.Translation);
            modelMatrix =
            glm::rotate(modelMatrix, renderable->Transform.Rotation.w,
                        { renderable->Transform.Rotation.x, renderable->Transform.Rotation.y,
                          renderable->Transform.Rotation.z });
            modelMatrix = glm::scale(modelMatrix, renderable->Transform.Scale); // cache ?

            usedShader->SetMatrix(modelMatrixUniformId, modelMatrix);

            // setup the shader's uniform using material's properties

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
        if (Shader == defaultShader)
        {
            Shader->SetFloat(ambientStrengthUniformId, ambientStrength);
            Shader->SetMatrix(projectionMatrixUniformId, Target->Camera->GetProjectionMatrix());
            Shader->SetMatrix(viewMatrixUniformId, Target->Camera->GetViewMatrix());
            Shader->SetVector(viewPositionUniformId, Target->Camera->Position);
            return;
        }

        int32_t ambientStrengthUniformId = Shader->GetIdForUniform(AMBIENT_STRENGTH);
        int32_t projectionMatrixUniformId = Shader->GetIdForUniform(PROJECTION_MATRIX);
        int32_t viewMatrixUniformId = Shader->GetIdForUniform(VIEW_MATRIX);
        int32_t viewPositionUniformId = Shader->GetIdForUniform(VIEW_POSITION);

        Shader->SetFloat(ambientStrengthUniformId, ambientStrength);
        Shader->SetMatrix(projectionMatrixUniformId, Target->Camera->GetProjectionMatrix());
        Shader->SetMatrix(viewMatrixUniformId, Target->Camera->GetViewMatrix());
        Shader->SetVector(viewPositionUniformId, Target->Camera->Position);
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

            //@TODO all uniform locations can be cached for the default shader
            uint32_t lightDirectionUniformId = Shader->GetIdForUniform(uniformNameBuffer);
            Shader->SetVector(lightDirectionUniformId, lightNode->Element->Direction);

            sprintf(uniformNameBuffer, DIRECTIONAL_LIGHT_COLOR, ligthIdx);

            uint32_t lightColorUniformId = Shader->GetIdForUniform(uniformNameBuffer);
            Shader->SetVector(lightColorUniformId, lightNode->Element->Color);

            lightNode = lightNode->Next;
        }

        uint32_t numOfDirectionalLightsUniformId = Shader->GetIdForUniform(NUM_OF_DIRECTIONAL_LIGHTS);

        Shader->SetInt(numOfDirectionalLightsUniformId, directionalLighsCount);
    }
} // namespace lucid::scene