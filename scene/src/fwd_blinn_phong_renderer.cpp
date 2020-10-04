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

    void ForwardBlinnPhongRenderer::Render(const RenderScene* SceneToRender, const RenderTarget* Target)
    {
        // temporary light for testing purposes //

        glm::vec3 lightPos{ 3, 4, 4 };

        //////////////////////////////////////////

        if (Target->Framebuffer)
        {
            Target->Framebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        }
        else
        {
            gpu::BindDefaultFramebuffer(gpu::FramebufferBindMode::READ_WRITE);
        }

        gpu::SetViewport(Target->Viewport);

        defaultShader->Use();
        defaultShader->SetFloat(AMBIENT_STRENGTH, ambientStrength);
        defaultShader->SetMatrix(PROJECTION_MATRIX, Target->Camera->GetProjectionMatrix());
        defaultShader->SetMatrix(VIEW_MATRIX, Target->Camera->GetViewMatrix());
        defaultShader->SetVector(LIGHT_POSITION, lightPos);
        defaultShader->SetVector(VIEW_POSITION, Target->Camera->Position);
        defaultShader->SetMatrix(MODEL_MATRIX, glm::mat4{ 1 });

        auto usedShader = defaultShader;
        auto currentNode = &SceneToRender->Renderables.Head;

        // TODO sort by shader in the material to avoid costly program switches

        while (currentNode && currentNode->Element)
        {
            auto renderable = currentNode->Element;
            LUCID_LOG(LogLevel::INFO, "Rendering '%s'", renderable->Name.CString);

            // material properties requried by the renderer
            glm::vec3* diffuseColor = nullptr;
            glm::vec3* specularColor = nullptr;

            // Determine if the material uses a custom shader
            // if yes, then setup the renderer-provied uniforms

            auto customShader = renderable->Material->GetCustomShader();
            if (customShader)
            {
                usedShader = customShader;
                usedShader->Use();
                usedShader->SetFloat(AMBIENT_STRENGTH, ambientStrength);
                usedShader->SetMatrix(PROJECTION_MATRIX, Target->Camera->GetProjectionMatrix());
                usedShader->SetMatrix(VIEW_MATRIX, Target->Camera->GetViewMatrix());
                usedShader->SetVector(LIGHT_POSITION, lightPos);
            }

            // setup the sahder's uniform using material's properties

            renderable->Material->SetupShader(usedShader);

            // bind the renderable's vertex array and draw it

            renderable->VertexArray->Bind();
            renderable->VertexArray->Draw();

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
} // namespace lucid::scene