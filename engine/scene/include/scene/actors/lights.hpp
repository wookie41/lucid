#pragma once

#include "scene/settings.hpp"
#include "scene/actors/actor.hpp"

#include "common/types.hpp"
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
    constexpr u8 MAX_SHADOW_CASCADES = 6;

    class CShadowMap;

    class CLight : public IActor
    {
      public:
        CLight(const FDString& InName, IActor* InParent, CWorld* InWorld) : IActor(InName, InParent, InWorld, math::FAABB {}){};

        virtual ELightType GetType() const = 0;

        /** Recalculates the light space matrix when e.x. the light moves or is initially created */
        virtual void UpdateLightSpaceMatrix(const LightSettings& LightSettings) = 0;

        /** Sets up the shader's uniform to use this light's data */
        virtual void SetupShader(gpu::CShader* InShader) const;
        virtual void SetupShadowMapShader(gpu::CShader* InShader) = 0;
        virtual void CreateShadowMap();

        static EActorType GetActorTypeStatic() { return EActorType::LIGHT; }

        virtual EActorType GetActorType() const override { return EActorType::LIGHT; }
        virtual IActor*    CreateAssetFromActor(const FDString& InName) const override
        {
            assert(0);
            return nullptr;
        }
        virtual IActor* CreateActorInstanceFromAsset(CWorld* InWorld, const glm::vec3& InSpawnPosition) override
        {
            assert(0);
            return nullptr;
        };

        virtual IActor* LoadActor(CWorld* InWorld, FActorEntry const* InActorDescription) override
        {
            assert(0);
            return nullptr;
        }
        void CleanupAfterRemove() override;

        glm::vec3 Color{ 1, 1, 1 };
        u8        Quality = 1;

        CShadowMap* ShadowMap = nullptr;

        bool bCastsShadow = false;

        virtual ~CLight() = default;

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

      protected:
        virtual void InternalSaveAssetToFile(const FString& InActorResourceName) override;

      public:
#endif
    };

    /////////////////////////////////////
    //        Directional light        //
    /////////////////////////////////////

    class CDirectionalLight : public CLight
    {
      public:
        CDirectionalLight(const FDString& InName, IActor* InParent, CWorld* InWorld) : CLight(InName, InParent, InWorld) {}

        virtual void    CreateShadowMap() override;
        virtual void    UpdateLightSpaceMatrix(const LightSettings& LightSettings) override;
        virtual void    SetupShader(gpu::CShader* InShader) const override;
        virtual void    SetupShadowMapShader(gpu::CShader* InShader) override;
        virtual IActor* CreateActorCopy() override;

        virtual void OnAddToWorld(CWorld* InWorld) override;
        virtual void OnRemoveFromWorld(const bool& InbHardRemove) override;

        virtual ELightType GetType() const override { return ELightType::DIRECTIONAL; }

        void OnRotated(const glm::quat& InOldRotation, const glm::quat& InNewRotation) override;

        glm::vec3 Direction = { 0, 0, -1 };
        glm::vec3 LightUp{ 0, 1, 0 };
        glm::mat4 LightSpaceMatrix{ 1 };

        float Illuminance = 10;

        float Left      = -10;
        float Right     = 10;
        float Bottom    = -10;
        float Top       = 10;
        float NearPlane = -10;
        float FarPlane  = 10000;

        u8    CascadeCount          = 3;
        float CascadeSplitLogFactor = 0.9f;
        float FirstCascadeNearPlane = 1.f;

        CShadowMap* CascadeShadowMaps[MAX_SHADOW_CASCADES]{ nullptr };

        float     CascadeFarPlanes[MAX_SHADOW_CASCADES];
        glm::mat4 CascadeMatrices[MAX_SHADOW_CASCADES];

#if DEVELOPMENT
        virtual void UIDrawActorDetails() override;
        virtual void OnSelectedPreFrameRender() override;
#endif
    };

    inline void CDirectionalLight::OnRotated(const glm::quat& in_old_rotation, const glm::quat& in_new_rotation)
    {
        CLight::OnRotated(in_old_rotation, in_new_rotation);

        Direction = glm::normalize(glm::vec3{ 0, 0, 1 } * GetTransform().Rotation);
    }

    /////////////////////////////////////
    //            Spot light           //
    /////////////////////////////////////

    class CSpotLight : public CLight
    {
      public:
        CSpotLight(const FDString& InName, IActor* InParent, CWorld* InWorld) : CLight(InName, InParent, InWorld) {}

        virtual ELightType GetType() const override { return ELightType::SPOT; }

        virtual void    UpdateLightSpaceMatrix(const LightSettings& LightSettings) override;
        virtual void    SetupShader(gpu::CShader* InShader) const override;
        virtual void    SetupShadowMapShader(gpu::CShader* InShader) override;
        virtual IActor* CreateActorCopy() override;

        virtual void OnAddToWorld(CWorld* InWorld) override;
        virtual void OnRemoveFromWorld(const bool& InbHardRemove) override;
        
        void OnRotated(const glm::quat& InOldRotation, const glm::quat& InNewRotation) override;
        
        glm::vec3 Direction{ 0, 0, -1 };
        glm::vec3 LightUp{ 0, 1, 0 };

        glm::mat4 LightSpaceMatrix{ 1 };

        float AttenuationRadius = 50;

        float InnerCutOffRad = 0.523598776; // 30 deg
        float OuterCutOffRad = 0.785398163; // 45 deg

        ELightSourceType LightSourceType = ELightSourceType::INCANDESCENT;

        /** Light unit in which intensity of this light is specified */
        ELightUnit LightUnit = ELightUnit::LUMENS;

        /** Luminous power in lumens, default corresponds to a light bulb */
        float LuminousPower = 620.f;

        /** Radiant power in Watts, default corresponds to a light bulb */
        float RadiantPower = 8.f;

#if DEVELOPMENT
        virtual void UIDrawActorDetails() override;
        virtual void OnSelectedPreFrameRender() override;
#endif
    };

    /////////////////////////////////////
    //           Point light           //
    /////////////////////////////////////

    class CPointLight : public CLight
    {
      public:
        CPointLight(const FDString& InName, IActor* InParent, CWorld* InWorld) : CLight(InName, InParent, InWorld){};
        virtual ELightType GetType() const override { return ELightType::POINT; }

        virtual void    UpdateLightSpaceMatrix(const LightSettings& LightSettings) override;
        virtual void    SetupShader(gpu::CShader* InShader) const override;
        virtual void    SetupShadowMapShader(gpu::CShader* InShader) override;
        virtual IActor* CreateActorCopy() override;

        virtual void OnAddToWorld(CWorld* InWorld) override;
        virtual void OnRemoveFromWorld(const bool& InbHardRemove) override;

        float AttenuationRadius = 50;

        glm::mat4 LightSpaceMatrices[6];

        float CachedNearPlane = 1.f;
        float CachedFarPlane  = 25.f;

        ELightSourceType LightSourceType = ELightSourceType::INCANDESCENT;

        /** Light unit in which intensity of this light is specified */
        ELightUnit LightUnit = ELightUnit::LUMENS;

        /** Luminous power in lumens, default corresponds to a light bulb */
        float LuminousPower = 620.f;

        /** Radiant power in Watts, default corresponds to a light bulb */
        float RadiantPower = 8.f;

#if DEVELOPMENT
        virtual void UIDrawActorDetails() override;
        virtual void OnSelectedPreFrameRender() override;
#endif
    };
} // namespace lucid::scene
