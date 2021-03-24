#pragma once

#include "common/types.hpp"
#include "devices/gpu/gpu.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class CShader;
    class CTexture;
    class CFramebuffer;
    class CCubemap;
} // namespace lucid::gpu

namespace lucid::scene
{
    struct FRenderScene;

    enum class ELightType : u8
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    struct FShadowMap
    {
        gpu::CTexture* ShadowMapTexture;
    };

    class CLight
    {
      public:
        CLight() = default;

        virtual ELightType GetType() const = 0;
        virtual ~CLight() = default;

        virtual void UpdateLightSpaceMatrix() = 0;

        // Updates the depth map by rendering the `SceneToRender` to the `TargetFramebuffer` using the `ShaderToUse`
        // assumes that `TargetFramebuffer` is bound
        // if `RenderStaticGeometry` is `false`, the it'll only render the dynamic geometry of the `RenderScene`
        virtual void GenerateShadowMap(FRenderScene* SceneToRender,
                                       gpu::CFramebuffer* TargetFramebuffer,
                                       gpu::CShader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap = false) = 0;

        glm::vec3 Position = { 0, 0, 0 };
        glm::vec3 Color = { 0, 0, 0 };
        glm::ivec2 ShadowMapSize{ 0, 0 };
    };

    class CDirectionalLight : public CLight
    {
      public:
        CDirectionalLight() = default;

        void UpdateLightSpaceMatrix() override;

        virtual void GenerateShadowMap(FRenderScene* SceneToRender,
                                       gpu::CFramebuffer* TargetFramebuffer,
                                       gpu::CShader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        virtual ELightType GetType() const override { return type; }

        glm::vec3 Direction = { 0, 0, 0 };

        glm::vec3 LightUp{ 0, 1, 0 };
        glm::mat4 LightSpaceMatrix{ 1 };
        gpu::CTexture* ShadowMap = nullptr;

      private:
        static const ELightType type = ELightType::DIRECTIONAL;
    };

    class CPointLight : public CLight
    {
      public:
        virtual ELightType GetType() const override { return type; }

        virtual void UpdateLightSpaceMatrix() override;

        virtual void GenerateShadowMap(FRenderScene* SceneToRender,
                                       gpu::CFramebuffer* TargetFramebuffer,
                                       gpu::CShader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        float Constant = 0;
        float Linear = 0;
        float Quadratic = 0;

        glm::mat4 LightSpaceMatrices[6];
        gpu::CCubemap* ShadowMap = nullptr;

        float NearPlane = 1.f;
        float FarPlane = 25.f;

      private:
        static const ELightType type = ELightType::POINT;
    };

    class CSpotLight : public CLight
    {
      public:
        virtual ELightType GetType() const { return type; }

        virtual void UpdateLightSpaceMatrix() override;

        virtual void GenerateShadowMap(FRenderScene* SceneToRender,
                                       gpu::CFramebuffer* TargetFramebuffer,
                                       gpu::CShader* ShaderToUse,
                                       bool RenderStaticGeometry,
                                       bool ClearShadowMap) override;

        glm::vec3 Direction = { 0, 0, 0 };

        glm::mat4 LightSpaceMatrix{ 1 };
        gpu::CTexture* ShadowMap = nullptr;

        glm::vec3 LightUp{ 0, 1, 0 };
        float Constant = 0;
        float Linear = 0;
        float Quadratic = 0;
        float InnerCutOffRad = 0;
        float OuterCutOffRad = 0;

      private:
        static const ELightType type = ELightType::SPOT;
    };

    // If CastsShadow = true, then this function will generate a shadow map for the light
    // if it's false, then the width and height parameters can be anything, but for you should make them 0 for clarity
    CDirectionalLight CreateDirectionalLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize, gpu::FGPUState* InGPUState);
    CSpotLight CreateSpotLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize, gpu::FGPUState* InGPUState);
    CPointLight CreatePointLight(const bool& CastsShadow, const glm::ivec2& ShadowMapSize, gpu::FGPUState* InGPUState);
} // namespace lucid::scene
