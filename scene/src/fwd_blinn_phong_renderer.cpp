#include "scene/fwd_blinn_phong_renderer.hpp"

#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/texture.hpp"

#include "scene/renderable.hpp"
#include "scene/camera.hpp"
#include "scene/blinn_phong_material.hpp"

#include "resources/texture.hpp"
#include "resources/mesh.hpp"

#include "common/log.hpp"

#include <stdio.h>

namespace lucid::scene
{
    // Shader-wide uniforms
    static const String AMBIENT_STRENGTH("uAmbientStrength");

    static const String VIEW_POSITION("uViewPos");

    static const String MODEL_MATRIX("uModel");
    static const String VIEW_MATRIX("uView");
    static const String PROJECTION_MATRIX("uProjection");

    static const String LIGHT_DIRECTION("uLight.Direction");
    static const String LIGHT_COLOR("uLight.Color");

    ForwardBlinnPhongRenderer::ForwardBlinnPhongRenderer(const uint32_t& MaxNumOfDirectionalLights, gpu::Shader* DefaultShader)
    : Renderer(DefaultShader), maxNumOfDirectionalLights(MaxNumOfDirectionalLights)
    {
    }

    void ForwardBlinnPhongRenderer::Render(const RenderScene* SceneToRender, const RenderTarget* Target)
    {
        SetupFramebuffer(Target->Framebuffer);
        gpu::SetViewport(Target->Viewport);

        defaultShader->Use();

        SetupRendererWideUniforms(defaultShader, Target);
        RenderStaticGeometry(SceneToRender, Target);
    }

    void ForwardBlinnPhongRenderer::RenderStaticGeometry(const RenderScene* SceneToRender, const RenderTarget* Target)
    {
        auto usedShader = defaultShader;
        auto lightNode = &SceneToRender->DirectionalLights.Head;
        while (lightNode && lightNode->Element)
        {
            auto currentNode = &SceneToRender->StaticGeometry.Head;

            while (currentNode && currentNode->Element)
            {
                auto renderable = currentNode->Element;
                // LUCID_LOG(LogLevel::INFO, "Rendering '%s'", (char const*)renderable->Name);

                // Determine if the material uses a custom shader
                // if yes, then setup the renderer-provided uniforms

                auto customShader = renderable->Material->GetCustomShader();
                if (customShader)
                {
                    usedShader = customShader;
                    customShader->Use();
                    SetupRendererWideUniforms(customShader, Target);
                }

                SetupDirectionalLight(usedShader, lightNode->Element);
                Render(usedShader, renderable);

                // if the renderable used a custom material, then restore the default one

                if (usedShader != defaultShader)
                {
                    usedShader = defaultShader;
                    defaultShader->Use();
                }

                currentNode = currentNode->Next;
            }

            lightNode = lightNode->Next;
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

    void ForwardBlinnPhongRenderer::SetupDirectionalLight(gpu::Shader* Shader, const DirectionalLight* Light)
    {
        Shader->SetVector(Shader->GetIdForUniform(LIGHT_DIRECTION), Light->Direction);
        Shader->SetVector(Shader->GetIdForUniform(LIGHT_COLOR), Light->Color);
    }

    inline glm::mat4 CalculateModelMatrix(const Transform3D& Transform)
    {
        glm::mat4 modelMatrix{ 1 };
        modelMatrix = glm::translate(modelMatrix, Transform.Translation);
        modelMatrix =
          glm::rotate(modelMatrix, Transform.Rotation.w, { Transform.Rotation.x, Transform.Rotation.y, Transform.Rotation.z });
        modelMatrix = glm::scale(modelMatrix, Transform.Scale);
        return modelMatrix;
    }

    void ForwardBlinnPhongRenderer::Render(gpu::Shader* Shader, Renderable* const ToRender)
    {
        glm::mat4 modelMatrix = CalculateModelMatrix(ToRender->Transform);
        Shader->SetMatrix(Shader->GetIdForUniform(MODEL_MATRIX), modelMatrix);

        ToRender->Material->SetupShader(Shader);

        ToRender->VertexArray->Bind();
        ToRender->VertexArray->Draw();
    }

} // namespace lucid::scene