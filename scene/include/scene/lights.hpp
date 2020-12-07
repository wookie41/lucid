#pragma once

#include "glm/glm.hpp"

namespace lucid::gpu
{
    class Shader;
    class Texture;
    class Framebuffer;
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
        Light(const glm::ivec2& ShadowMapSize = { 0, 0 }, gpu::Texture* ShadowMap = nullptr);

        virtual LightType GetType() = 0;
        virtual ~Light() = default;

        // Updates the depth map by rendering the `SceneToRender` to the `TargetFramebuffer` using the `ShaderToUse`
        // assumes that `TargetFramebuffer` is bound
        // if `RenderStaticGeometry` is `false`, the it'll only render the dynamic geometry of the `RenderScene`
        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap = false) = 0;

        inline gpu::Texture* GetCachedShadowMap() { return shadowMap; };
        inline glm::ivec2 GetShadowMapSize() { return shadowMapSize; };

        glm::vec3 Position = {0, 0, 0};        
        
      protected:
        gpu::Texture* shadowMap = nullptr;
        glm::ivec2 shadowMapSize;
    };

    class DirectionalLight : public Light
    {
      public:
        DirectionalLight(const glm::ivec2& ShadowMapSize = { 0, 0 }, gpu::Texture* ShadowMap = 0);

        void UpdateLightSpaceMatrix();

        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        virtual LightType GetType() { return type; }

        glm::vec3 Direction = { 0, 0, 0 };
        glm::vec3 Color = { 0, 0, 0 };
        
        glm::mat4 LightSpaceMatrix { 1 };
        glm::vec3 LightUp { 0, 1, 0 };

      private:
        static const LightType type = LightType::DIRECTIONAL;
    };

    struct PointLight : public Light
    {
      public:
        virtual LightType GetType() { return type; }

        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override {}

        float Constant = 0;
        float Linear = 0;
        float Quadratic = 0;
        glm::vec3 Color = { 0, 0, 0 };

      private:
        static const LightType type = LightType::POINT;
    };

    struct SpotLight : public PointLight
    {
      public:
        virtual LightType GetType() { return type; }


        virtual void GenerateShadowMap(RenderScene* SceneToRender,
                                       gpu::Framebuffer* TargetFramebuffer,
                                       gpu::Shader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override {};

        glm::vec3 Direction = { 0, 0, 0 };
        float InnerCutOffRad = 0;
        float OuterCutOffRad = 0;

      private:
        static const LightType type = LightType::SPOT;
    };

    // If CastsShadow = true, then this function will generate a shadow map for the light
    // if it's false, then the width and height parameters can be anything, but for you should make them 0 for clarity
    DirectionalLight CreateDirectionalLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize);
} // namespace lucid::scene
