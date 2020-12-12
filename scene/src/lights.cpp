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
    static const String MODEL{ "uModel" };
    static const String LIGHT_SPACE_MATRIX{ "uLightSpaceMatrix" };

    // Shoule be tweakable based on game's graphics settings
    struct OrthoMatrixLightSettings
    {
        float Left, Right;
        float Bottom, Top;
        float Near, Far;
    };

    static OrthoMatrixLightSettings LIGHT_SETTINGS = { -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 15.f };

    void RenderGeometry(gpu::Shader* ShaderToUse, LinkedList<class Renderable>& Geometry);

    inline glm::mat4 CreateLightSpaceMatrix(const glm::vec3& Position, const glm::vec3& LightUp)
    {
        glm::mat4 lightSpaceMatrix = glm::lookAt(Position, glm::vec3{ 0 }, LightUp);
        lightSpaceMatrix = glm::ortho(LIGHT_SETTINGS.Left, LIGHT_SETTINGS.Right, LIGHT_SETTINGS.Bottom, LIGHT_SETTINGS.Top,
                                      LIGHT_SETTINGS.Near, LIGHT_SETTINGS.Far) *
                           lightSpaceMatrix;
        return lightSpaceMatrix;
    }

    void DirectionalLight::UpdateLightSpaceMatrix() { LightSpaceMatrix = CreateLightSpaceMatrix(Position, LightUp); }

    static gpu::Texture* CreateShadowMapTexture(const glm::ivec2& ShadowMapSize)
    {
        gpu::Texture* shadowMap = gpu::CreateEmpty2DTexture(ShadowMapSize.x, ShadowMapSize.y, gpu::TextureDataType::FLOAT,
                                                            gpu::TextureFormat::DEPTH_COMPONENT, 0);
        shadowMap->Bind();
        shadowMap->SetWrapSFilter(lucid::gpu::WrapTextureFilter::CLAMP_TO_EDGE);
        shadowMap->SetWrapTFilter(lucid::gpu::WrapTextureFilter::CLAMP_TO_EDGE);
        shadowMap->SetMinFilter(lucid::gpu::MinTextureFilter::NEAREST);
        shadowMap->SetMagFilter(lucid::gpu::MagTextureFilter::NEAREST);

        assert(shadowMap);
        return shadowMap;
    }

    DirectionalLight CreateDirectionalLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize)
    {
        DirectionalLight directionalLight;
        directionalLight.ShadowMapSize = ShadowMapSize;

        if (CastsShadow)
        {
            directionalLight.ShadowMap = CreateShadowMapTexture(ShadowMapSize);
        }

        return directionalLight;
    }

    SpotLight CreateSpotLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize)
    {
        SpotLight spotLight;
        spotLight.ShadowMapSize = ShadowMapSize;

        if (CastsShadow)
        {
            spotLight.ShadowMap = CreateShadowMapTexture(ShadowMapSize);
        }

        return spotLight;
    }

    static void _GenerateShadowMap(RenderScene* SceneToRender,
                                   gpu::Framebuffer* TargetFramebuffer,
                                   gpu::Shader* ShaderToUse,
                                   bool RenderStaticGeometry,
                                   bool ClearShadowMap,
                                   gpu::Texture* ShadowMap,
                                   const glm::mat4& LightSpaceMatrix)
    {
        assert(ShadowMap);

        gpu::EnableDepthTest();
        gpu::SetDepthTestFunction(gpu::DepthTestFunction::LEQUAL);

        gpu::DisableBlending();

        TargetFramebuffer->Bind(gpu::FramebufferBindMode::READ_WRITE);
        TargetFramebuffer->SetupDepthAttachment(ShadowMap);

        if (ClearShadowMap)
        {
            gpu::ClearBuffers(gpu::ClearableBuffers::DEPTH);
        }

        TargetFramebuffer->DisableReadWriteBuffers();

        gpu::SetViewport({ 0, 0, (uint32_t)ShadowMap->GetSize().x, (uint32_t)ShadowMap->GetSize().y });

        ShaderToUse->Use();
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);

        gpu::SetCullMode(gpu::CullMode::FRONT);

        if (RenderStaticGeometry)
        {
            RenderGeometry(ShaderToUse, SceneToRender->StaticGeometry);
        }

        RenderGeometry(ShaderToUse, SceneToRender->DynamicGeometry);
    }

    void RenderGeometry(gpu::Shader* ShaderToUse, LinkedList<class Renderable>& Geometry)
    {
        LinkedListItem<Renderable>* node = &Geometry.Head;
        while (node && node->Element)
        {
            ShaderToUse->SetMatrix(MODEL, node->Element->CalculateModelMatrix());
            node->Element->VertexArray->Bind();
            node->Element->VertexArray->Draw();
            node = node->Next;
        }
    }

    void DirectionalLight::GenerateShadowMap(RenderScene* SceneToRender,
                                             gpu::Framebuffer* TargetFramebuffer,
                                             gpu::Shader* ShaderToUse,
                                             bool RenderStaticGeometry,
                                             bool ClearShadowMap)
    {
        _GenerateShadowMap(SceneToRender, TargetFramebuffer, ShaderToUse, RenderStaticGeometry, ClearShadowMap, ShadowMap,
                           LightSpaceMatrix);
    }

    void SpotLight::GenerateShadowMap(RenderScene* SceneToRender,
                                      gpu::Framebuffer* TargetFramebuffer,
                                      gpu::Shader* ShaderToUse,
                                      bool RenderStaticGeometry,
                                      bool ClearShadowMap)

    {
        _GenerateShadowMap(SceneToRender, TargetFramebuffer, ShaderToUse, RenderStaticGeometry, ClearShadowMap, ShadowMap,
                           LightSpaceMatrix);
    }

    void SpotLight::UpdateLightSpaceMatrix() { LightSpaceMatrix = CreateLightSpaceMatrix(Position, LightUp); }


} // namespace lucid::scene