#include "scene/lights.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/shader.hpp"
#include "scene/render_scene.hpp"
#include "scene/renderable.hpp"
#include "devices/gpu/vao.hpp"
#include "devices/gpu/viewport.hpp"
#include "devices/gpu/cubemap.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace lucid::scene
{
    static const String MODEL{ "uModel" };

    static const String LIGHT_SPACE_MATRIX{ "uLightSpaceMatrix" };

    static const String LIGHT_POSITION{ "uLightPosition" };
    static const String LIGHT_FAR_PLANE{ "uFarPlane" };
    static const String LIGHT_SPACE_MATRIX_0{ "uLightSpaceMatrices[0]" };
    static const String LIGHT_SPACE_MATRIX_1{ "uLightSpaceMatrices[1]" };
    static const String LIGHT_SPACE_MATRIX_2{ "uLightSpaceMatrices[2]" };
    static const String LIGHT_SPACE_MATRIX_3{ "uLightSpaceMatrices[3]" };
    static const String LIGHT_SPACE_MATRIX_4{ "uLightSpaceMatrices[4]" };
    static const String LIGHT_SPACE_MATRIX_5{ "uLightSpaceMatrices[5]" };

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

        if (CastsShadow)
        {
            directionalLight.ShadowMap = CreateShadowMapTexture(ShadowMapSize);
            directionalLight.ShadowMapSize = ShadowMapSize;
        }

        return directionalLight;
    }

    SpotLight CreateSpotLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize)
    {
        SpotLight spotLight;

        if (CastsShadow)
        {
            spotLight.ShadowMap = CreateShadowMapTexture(ShadowMapSize);
            spotLight.ShadowMapSize = ShadowMapSize;
        }

        return spotLight;
    }

    PointLight CreatePointLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize)
    {
        PointLight pointLight;

        if (CastsShadow)
        {
            pointLight.ShadowMap = gpu::CreateCubemap(ShadowMapSize, gpu::TextureFormat::DEPTH_COMPONENT,
                                                      gpu::TextureFormat::DEPTH_COMPONENT, gpu::TextureDataType::FLOAT);
            pointLight.ShadowMapSize = ShadowMapSize;
        }

        return pointLight;
    }

    static void _GenerateShadowMap(RenderScene* SceneToRender,
                                   gpu::Framebuffer* TargetFramebuffer,
                                   gpu::Shader* ShaderToUse,
                                   bool RenderStaticGeometry,
                                   bool ClearShadowMap,
                                   gpu::Texture* ShadowMap)
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

        // gpu::SetCullMode(gpu::CullMode::FRONT);
        gpu::DisableCullFace();

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
        ShaderToUse->Use();
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);

        _GenerateShadowMap(SceneToRender, TargetFramebuffer, ShaderToUse, RenderStaticGeometry, ClearShadowMap, ShadowMap);
    }

    void DirectionalLight::UpdateLightSpaceMatrix() { LightSpaceMatrix = CreateLightSpaceMatrix(Position, LightUp); }

    void SpotLight::GenerateShadowMap(RenderScene* SceneToRender,
                                      gpu::Framebuffer* TargetFramebuffer,
                                      gpu::Shader* ShaderToUse,
                                      bool RenderStaticGeometry,
                                      bool ClearShadowMap)

    {
        ShaderToUse->Use();
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);

        _GenerateShadowMap(SceneToRender, TargetFramebuffer, ShaderToUse, RenderStaticGeometry, ClearShadowMap, ShadowMap);
    }

    void SpotLight::UpdateLightSpaceMatrix() { LightSpaceMatrix = CreateLightSpaceMatrix(Position, LightUp); }

    void PointLight::UpdateLightSpaceMatrix()
    {
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.f), (float)ShadowMapSize.x / (float)ShadowMapSize.y, NearPlane, FarPlane);

        LightSpaceMatrices[0] = projectionMatrix * glm::lookAt(Position, Position + glm::vec3 { 1.0, 0.0, 0.0 }, glm::vec3 { 0.0, -1.0, 0.0 });
        LightSpaceMatrices[1] = projectionMatrix * glm::lookAt(Position, Position + glm::vec3 { -1.0, 0.0, 0.0 }, glm::vec3 { 0.0, -1.0, 0.0 });
        LightSpaceMatrices[2] = projectionMatrix * glm::lookAt(Position, Position + glm::vec3 { 0.0, 1.0, 0.0 }, glm::vec3 { 0.0, 0.0, 1.0 });
        LightSpaceMatrices[3] = projectionMatrix * glm::lookAt(Position, Position + glm::vec3 { 0.0, -1.0, 0.0 }, glm::vec3 { 0.0, 0.0, -1.0 });
        LightSpaceMatrices[4] = projectionMatrix * glm::lookAt(Position, Position + glm::vec3 { 0.0, 0.0, 1.0 }, glm::vec3 { 0.0, -1.0, 0.0 });
        LightSpaceMatrices[5] = projectionMatrix * glm::lookAt(Position, Position + glm::vec3 { 0.0, 0.0, -1.0 }, glm::vec3 { 0.0, -1.0, 0.0 });
    }

    void PointLight::GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap)

    {
        ShaderToUse->Use();
        ShaderToUse->SetFloat(LIGHT_FAR_PLANE, FarPlane);
        ShaderToUse->SetVector(LIGHT_POSITION, Position);
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX_0, LightSpaceMatrices[0]);
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX_1, LightSpaceMatrices[1]);
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX_2, LightSpaceMatrices[2]);
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX_3, LightSpaceMatrices[3]);
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX_4, LightSpaceMatrices[4]);
        ShaderToUse->SetMatrix(LIGHT_SPACE_MATRIX_5, LightSpaceMatrices[5]);

        _GenerateShadowMap(SceneToRender, TargetFramebuffer, ShaderToUse, RenderStaticGeometry, ClearShadowMap, ShadowMap);
    }

} // namespace lucid::scene