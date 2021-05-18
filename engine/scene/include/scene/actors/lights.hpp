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
    class CShadowMap;

    enum class ELightType : u8
    {
        DIRECTIONAL = 1,
        POINT,
        SPOT
    };

    class CLight : public IActor
    {
      public:

        CLight(const FDString& InName, IActor* InParent, CWorld* InWorld) : IActor(InName, InParent, InWorld)
        {
        };

        virtual ELightType GetType() const = 0;
        
        /** Recalculates the light space matrix when e.x. the light moves or is initially created */
        virtual void UpdateLightSpaceMatrix(const LightSettings& LightSettings) = 0;

        /** Sets up the shader's uniform to use this light's data */
        virtual void SetupShader(gpu::CShader* InShader) const;
        virtual void SetupShadowMapShader(gpu::CShader* InShader) = 0;

        virtual float GetVerticalMidPoint() const override;

        static  EActorType  GetActorTypeStatic() { return EActorType::LIGHT; }

        virtual EActorType  GetActorType() const override { return EActorType::LIGHT; }
        virtual IActor*     CreateActorAsset(const FDString& InName) const override { assert(0); return nullptr; }
        virtual IActor*     CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition) override { assert(0); return nullptr; };

        void CleanupAfterRemove() override;

        glm::vec3       Color           { 1, 1, 1 };
        u8              Quality         = 1;

        CShadowMap*     ShadowMap       = nullptr;

        bool            bShouldCastShadow = false;

        virtual ~CLight() = default;

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;
    protected:
        virtual void InternalSaveToResourceFile(const FString& InActorResourceName) override;
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

        virtual void        UpdateLightSpaceMatrix(const LightSettings& LightSettings) override;
        virtual void        SetupShader(gpu::CShader* InShader) const override;
        virtual void        SetupShadowMapShader(gpu::CShader* InShader) override;
        virtual IActor*     CreateCopy() override;

        virtual void        OnAddToWorld(CWorld* InWorld) override;
        virtual void        OnRemoveFromWorld(const bool& InbHardRemove) override;

        virtual ELightType GetType() const override { return ELightType::DIRECTIONAL; }

        glm::vec3 Direction = { 0, 0, -1 };
        glm::vec3 LightUp{ 0, 1, 0 };
        glm::mat4 LightSpaceMatrix{ 1 };
        
        CShadowMap* ShadowMap = nullptr;

#if DEVELOPMENT
        virtual void UIDrawActorDetails() override;
#endif  
    };

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
        virtual IActor* CreateCopy() override;

        virtual void    OnAddToWorld(CWorld* InWorld) override;
        virtual void    OnRemoveFromWorld(const bool& InbHardRemove) override;

        glm::vec3 Direction { 0, 0, -1 };
        glm::vec3 LightUp   { 0, 1, 0 };

        glm::mat4 LightSpaceMatrix { 1 };

        float Constant  = 0.2;
        float Linear    = 0.1;
        float Quadratic = 1/6;
        float InnerCutOffRad = 0.523598776; // 30 deg
        float OuterCutOffRad = 0.785398163; // 45 deg

#if DEVELOPMENT
        virtual void UIDrawActorDetails() override;
#endif  
    };

    /////////////////////////////////////
    //           Point light           //
    /////////////////////////////////////

    class CPointLight : public CLight
    {
      public:

        CPointLight(const FDString& InName, IActor* InParent, CWorld* InWorld) : CLight(InName, InParent, InWorld) {}
        
        virtual ELightType GetType() const override { return ELightType::POINT; }

        virtual void    UpdateLightSpaceMatrix(const LightSettings& LightSettings) override;
        virtual void    SetupShader(gpu::CShader* InShader) const override;
        virtual void    SetupShadowMapShader(gpu::CShader* InShader) override;
        virtual IActor* CreateCopy() override;

        virtual void    OnAddToWorld(CWorld* InWorld) override;
        virtual void    OnRemoveFromWorld(const bool& InbHardRemove) override;

        float Constant  = 0.2;
        float Linear    = 0.1;
        float Quadratic = 0.015;

        glm::mat4 LightSpaceMatrices[6];

        float CachedNearPlane = 1.f;
        float CachedFarPlane  = 25.f;

#if DEVELOPMENT
        virtual void UIDrawActorDetails() override;
#endif  
    };
} // namespace lucid::scene
