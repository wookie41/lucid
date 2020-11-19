#include "scene/lights.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/shader.hpp"
#include "scene/render_scene.hpp"
#include "scene/renderable.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/viewport.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace lucid::scene
{
    // Shoule be tweakable based on game's graphics settings
    struct OrthoMatrixLightSettings
    {
        float Left, Right;
        float Bottom, Top;
        float Near, Far;
    };

    static OrthoMatrixLightSettings LIGHT_SETTINGS = { -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f };

    void RenderGeometry(gpu::Shader* ShaderToUse, LinkedList<class Renderable>& Geometry, const uint32_t& ModelMatrixId);

    void DirectionalLight::UpdateLightSpaceMatrix()
    {
        LightSpaceMatrix = glm::lookAt(Position, Direction, LightUp);
        LightSpaceMatrix = glm::ortho(LIGHT_SETTINGS.Left, LIGHT_SETTINGS.Right, LIGHT_SETTINGS.Bottom, LIGHT_SETTINGS.Top, LIGHT_SETTINGS.Near, LIGHT_SETTINGS.Far) * LightSpaceMatrix;
    }

    DirectionalLight CreateDirectionalLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize)
    {
        gpu::Texture* shadowMap = nullptr;
        if (CastsShadow)
        {
            shadowMap = gpu::CreateEmpty2DTexture(ShadowMapSize.x, ShadowMapSize.y, gpu::TextureDataType::FLOAT,
                                                  gpu::TextureFormat::DEPTH_COMPONENT, 0);
            assert(shadowMap);
        }

        return { ShadowMapSize, shadowMap };
    }

    Light::Light(const glm::ivec2& ShadowMapSize, gpu::Texture* ShadowMap) : shadowMapSize(ShadowMapSize), shadowMap(ShadowMap) {}

    DirectionalLight::DirectionalLight(const glm::ivec2& ShadowMapSize, gpu::Texture* ShadowMap) : Light(ShadowMapSize, ShadowMap)
    {
    }

    void DirectionalLight::GenerateShadowMap(RenderScene* SceneToRender,
                                             gpu::Framebuffer* TargetFramebuffer,
                                             gpu::Shader* ShaderToUse,
                                             bool RenderStaticGeometry,
                                             bool ClearShadowMap)
    {
        assert(shadowMap);

        gpu::EnableDepthTest();
        gpu::DisableBlending();
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        TargetFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        TargetFramebuffer->SetupDepthAttachment(shadowMap);

        if (ClearShadowMap)
        {
            gpu::ClearBuffers(gpu::ClearableBuffers::DEPTH);
        }

        TargetFramebuffer->DisableReadWriteBuffers();

        gpu::SetViewport({ 0, 0, (uint32_t)shadowMap->GetSize().x, (uint32_t)shadowMap->GetSize().y });

        int32_t modelMatrixId = ShaderToUse->GetIdForUniform("uModel");
        int32_t LightSpaceMatrixId = ShaderToUse->GetIdForUniform("uLightSpaceMatrix");

        ShaderToUse->Use();
        ShaderToUse->SetMatrix(LightSpaceMatrixId, LightSpaceMatrix);

        if (RenderStaticGeometry)
        {
            RenderGeometry(ShaderToUse, SceneToRender->StaticGeometry, modelMatrixId);
        }

        RenderGeometry(ShaderToUse, SceneToRender->DynamicGeometry, modelMatrixId);
    }

    void RenderGeometry(gpu::Shader* ShaderToUse, LinkedList<class Renderable>& Geometry, const uint32_t& ModelMatrixId)
    {
        LinkedListItem<Renderable>* node = &Geometry.Head;
        while (node && node->Element)
        {
            ShaderToUse->SetMatrix(ModelMatrixId, node->Element->CalculateModelMatrix());
            node->Element->VertexArray->Bind();
            node->Element->VertexArray->Draw();
            node = node->Next;
        }
    }

} // namespace lucid::scene