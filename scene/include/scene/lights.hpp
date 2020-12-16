#pragma once

#include "glm/glm.hpp"

namespace lucid::gpu
{
    class Shader;
    class Texture;
    class Framebuffer;
    class Cubemap;
} // namespace lucid::gpu

namespace lucid::scene
{
    class RenderScene;

    enum class LightType : uint8_t
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    struct ShadowMap
    {
        gpu::Texture* ShadowMapTexture;
    };

    class Light
    {
      public:
        Light() = default;

        virtual LightType GetType() = 0;
        virtual ~Light() = default;

        virtual void UpdateLightSpaceMatrix() = 0;

        // Updates the depth map by rendering the `SceneToRender` to the `TargetFramebuffer` using the `ShaderToUse`
        // assumes that `TargetFramebuffer` is bound
        // if `RenderStaticGeometry` is `false`, the it'll only render the dynamic geometry of the `RenderScene`
        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap = false) = 0;

        glm::vec3 Position = { 0, 0, 0 };
        glm::vec3 Color = { 0, 0, 0 };
        glm::ivec2 ShadowMapSize{ 0, 0 };
    };

    class DirectionalLight : public Light
    {
      public:
        DirectionalLight() = default;

        void UpdateLightSpaceMatrix() override;

        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        virtual LightType GetType() override { return type; }

        glm::vec3 Direction = { 0, 0, 0 };

        glm::vec3 LightUp{ 0, 1, 0 };
        glm::mat4 LightSpaceMatrix{ 1 };
        gpu::Texture* ShadowMap = nullptr;

      private:
        static const LightType type = LightType::DIRECTIONAL;
    };

    struct PointLight : public Light
    {
      public:
        virtual LightType GetType() override { return type; }

        virtual void UpdateLightSpaceMatrix() override;

        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        float Constant = 0;
        float Linear = 0;
        float Quadratic = 0;

        glm::mat4 LightSpaceMatrices[6];
        gpu::Cubemap* ShadowMap = nullptr;

        float NearPlane = 1.f;
        float FarPlane = 25.f;

      private:
        static const LightType type = LightType::POINT;
    };

    struct SpotLight : public Light
    {
      public:
        virtual LightType GetType() { return type; }

        virtual void UpdateLightSpaceMatrix() override;

        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        glm::vec3 Direction = { 0, 0, 0 };

        glm::mat4 LightSpaceMatrix{ 1 };
        gpu::Texture* ShadowMap = nullptr;

        glm::vec3 LightUp{ 0, 1, 0 };
        float Constant = 0;
        float Linear = 0;
        float Quadratic = 0;
        float InnerCutOffRad = 0;
        float OuterCutOffRad = 0;

      private:
        static const LightType type = LightType::SPOT;
    };

    // If CastsShadow = true, then this function will generate a shadow map for the light
    // if it's false, then the width and height parameters can be anything, but for you should make them 0 for clarity
    DirectionalLight CreateDirectionalLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize);
    SpotLight CreateSpotLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize);
    PointLight CreatePointLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize);
} // namespace lucid::scene
