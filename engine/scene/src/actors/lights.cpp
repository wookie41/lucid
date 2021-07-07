#include "scene/actors/lights.hpp"

#include "scene/world.hpp"
#include "engine/engine.hpp"

#include "devices/gpu/texture.hpp"
#include "devices/gpu/shader.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "scene/renderer.hpp"
#include "scene/settings.hpp"

namespace lucid::scene
{
    static const FSString LIGHT_TYPE("uLightType");

    static const FSString LIGHT_POSITION{ "uLightPosition" };
    static const FSString LIGHT_COLOR{ "uLightColor" };

    static const FSString LIGHT_SPACE_MATRIX("uLightMatrix");

    static const FSString LIGHT_NEAR_PLANE{ "uLightNearPlane" };
    static const FSString LIGHT_FAR_PLANE{ "uLightFarPlane" };

    static const FSString LIGHT_SPACE_MATRIX_0{ "uLightMatrices[0]" };
    static const FSString LIGHT_SPACE_MATRIX_1{ "uLightMatrices[1]" };
    static const FSString LIGHT_SPACE_MATRIX_2{ "uLightMatrices[2]" };
    static const FSString LIGHT_SPACE_MATRIX_3{ "uLightMatrices[3]" };
    static const FSString LIGHT_SPACE_MATRIX_4{ "uLightMatrices[4]" };
    static const FSString LIGHT_SPACE_MATRIX_5{ "uLightMatrices[5]" };

    static const FSString LIGHT_DIRECTION("uLightDirection");
    static const FSString ATTENUATION_RADIUS("uLightAttenuationRadius");
    static const FSString LIGHT_INNER_CUT_OFF("uLightInnerCutOffCos");
    static const FSString LIGHT_OUTER_CUT_OFF("uLightOuterCutOffCos");
    static const FSString LIGHT_SHADOW_MAP("uLightShadowMap");
    static const FSString LIGHT_CASTS_SHADOWS("uLightCastsShadows");
    static const FSString LIGHT_SHADOW_CUBE("uLightShadowCube");

    void CLight::SetupShader(gpu::CShader* InShader) const
    {
        InShader->SetInt(LIGHT_TYPE, static_cast<u32>(GetType()));
        InShader->SetVector(LIGHT_POSITION, Transform.Translation);
        InShader->SetVector(LIGHT_COLOR, Color);
    }

#if DEVELOPMENT
    void CLight::UIDrawActorDetails()
    {
        IActor::UIDrawActorDetails();
        if (ImGui::CollapsingHeader("Light"))
        {
            switch (GetType())
            {
            case ELightType::DIRECTIONAL:
                ImGui::Text("Light type: Directional light");
                break;
            case ELightType::SPOT:
                ImGui::Text("Light type: Spot light");
                break;
            case ELightType::POINT:
                ImGui::Text("Light type: Point light");
                break;
            }

            ImGui::DragFloat3("Light color", &Color.r, 0.01, 0, 1);

            bool bCastsShadow = ShadowMap != nullptr;
            ImGui::Checkbox("Casts shadow", &bCastsShadow);
            if (bCastsShadow)
            {
                if (!ShadowMap)
                {
                    ShadowMap = GEngine.GetRenderer()->CreateShadowMap(GetType());
                }
            }
            else if (ShadowMap)
            {
                ShadowMap->Free();
                ShadowMap = nullptr;
            }
        }
    }
#endif

    float CLight::GetVerticalMidPoint() const { return 0; }

    void CLight::InternalSaveAssetToFile(const FString& InFilePath)
    {
        // Light data is written directly to the world file
    }

    void CLight::CleanupAfterRemove()
    {
        if (ShadowMap)
        {
            ShadowMap->Free();
            delete ShadowMap;
            ShadowMap = nullptr;
        }
    }

    /////////////////////////////////////
    //        Directional light        //
    /////////////////////////////////////

    void CDirectionalLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        const glm::mat4 ViewMatrix = glm::lookAt(Transform.Translation, Transform.Translation + Direction, LightUp);
        const glm::mat4 ProjectionMatrix =
          glm::ortho(LightSettings.Left, LightSettings.Right, LightSettings.Bottom, LightSettings.Top, LightSettings.Near, LightSettings.Far);

        LightSpaceMatrix = ProjectionMatrix * ViewMatrix;
    }

    void CDirectionalLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);
        InShader->SetVector(LIGHT_DIRECTION, Direction);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);

        if (ShadowMap != nullptr)
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
            InShader->UseTexture(LIGHT_SHADOW_MAP, ShadowMap->GetShadowMapTexture());
        }
        else
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
        }
    }

    void CDirectionalLight::SetupShadowMapShader(gpu::CShader* InShader) { InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix); }

#if DEVELOPMENT
    void CDirectionalLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();
        if (ImGui::CollapsingHeader("Directional light"))
        {
            ImGui::DragFloat3("Light up", &LightUp.x, 0.001, -1, 1);
        }

        if (bRotationUpdated)
        {
            Direction = glm::normalize(glm::vec3{ 0, 0, 1 } * Transform.Rotation);
        }
    }

    void CDirectionalLight::OnSelectedPreFrameRender()
    {
        constexpr glm::vec3 ArrowsOffsets[] = {
            { 0, 0, 0 },
            { 0, 2, 0 },
            { 0, -3, 0 },
        };

        const FDebugArrow DebugArrow = MakeDebugArrowData(Transform.Translation, Direction, 20);

        for (int i = 0; i < sizeof(ArrowsOffsets) / sizeof(glm::vec3); ++i)
        {
            GEngine.GetRenderer()->AddDebugLine(DebugArrow.BodyStart + ArrowsOffsets[i], DebugArrow.BodyEnd + ArrowsOffsets[i], Color, Color);
            GEngine.GetRenderer()->AddDebugLine(DebugArrow.HeadStart0 + ArrowsOffsets[i], DebugArrow.HeadEnd0 + ArrowsOffsets[i], Color, Color);
            GEngine.GetRenderer()->AddDebugLine(DebugArrow.HeadStart1 + ArrowsOffsets[i], DebugArrow.HeadEnd1 + ArrowsOffsets[i], Color, Color);
        }
    }
#endif

    IActor* CDirectionalLight::CreateActorCopy()
    {
        auto* Copy      = new CDirectionalLight{ Name.GetCopy(), Parent, World };
        Copy->Direction = Direction;
        Copy->Color     = Color;
        Copy->LightUp   = LightUp;
        Copy->Quality   = Quality;
        Copy->Transform = Transform;
        Copy->Transform.Translation += glm::vec3{ 1, 0, 0 };
        Copy->bShouldCastShadow = bShouldCastShadow;
        if (ShadowMap)
        {
            ShadowMap = GEngine.GetRenderer()->CreateShadowMap(ELightType::DIRECTIONAL);
        }
        World->AddDirectionalLight(Copy);
        return Copy;
    }

    void CDirectionalLight::OnAddToWorld(CWorld* InWorld)
    {
        CLight::OnAddToWorld(InWorld);
        InWorld->AddDirectionalLight(this);
        if (bShouldCastShadow && !ShadowMap)
        {
            ShadowMap = GEngine.GetRenderer()->CreateShadowMap(ELightType::DIRECTIONAL);
        }
    }

    void CDirectionalLight::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        CLight::OnRemoveFromWorld(InbHardRemove);
        World->RemoveDirectionalLight(ActorId);
        if (InbHardRemove)
        {
            CleanupAfterRemove();
        }
    }

    /////////////////////////////////////
    //            Spot light           //
    /////////////////////////////////////

    void CSpotLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        const float ShadowMapWidth  = (float)ShadowMap->GetShadowMapTexture()->GetWidth();
        const float ShadowMapHeight = (float)ShadowMap->GetShadowMapTexture()->GetHeight();

        const glm::mat4 ViewMatrix = glm::lookAt(Transform.Translation, Transform.Translation + Direction, LightUp);
        const glm::mat4 ProjectionMatrix =
          glm::perspective(OuterCutOffRad * 2, ShadowMapWidth / ShadowMapHeight, LightSettings.Near, LightSettings.Far);
        LightSpaceMatrix = ProjectionMatrix * ViewMatrix;
    }

    void CSpotLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);
        InShader->SetVector(LIGHT_DIRECTION, Direction);
        InShader->SetFloat(ATTENUATION_RADIUS, AttenuationRadius);
        InShader->SetFloat(LIGHT_INNER_CUT_OFF, glm::cos(InnerCutOffRad));
        InShader->SetFloat(LIGHT_OUTER_CUT_OFF, glm::cos(OuterCutOffRad));
        InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);

        if (ShadowMap)
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
            InShader->UseTexture(LIGHT_SHADOW_MAP, ShadowMap->GetShadowMapTexture());
        }
        else
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
        }
    }

    void CSpotLight::SetupShadowMapShader(gpu::CShader* InShader) { InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix); }

#if DEVELOPMENT
    void CSpotLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();
        if (ImGui::CollapsingHeader("Spot light"))
        {
            ImGui::DragFloat3("Light up", &LightUp.x, 0.001, -1, 1);

            float InnerCutOff = glm::degrees(InnerCutOffRad);
            float OuterCutOff = glm::degrees(OuterCutOffRad);

            ImGui::DragFloat("Attenuation radius", &AttenuationRadius, 1, 0, 10000);
            if (ImGui::DragFloat("Inner Cut Off", &InnerCutOff, 1, 0, 90))
            {
                if (InnerCutOff < OuterCutOff)
                {
                    InnerCutOffRad = glm::radians(InnerCutOff);
                }
            }
            if (ImGui::DragFloat("Outer Cut Off", &OuterCutOff, 1, 0, 90))
            {
                if (OuterCutOff > InnerCutOff)
                {
                    OuterCutOffRad = glm::radians(OuterCutOff);
                }
            }
        }

        if (bRotationUpdated)
        {
            Direction = glm::normalize(glm::vec3{ 0, 0, 1 } * Transform.Rotation);
            LightUp   = glm::normalize(glm::vec3{ 0, 1, 0 } * Transform.Rotation);
        }
    }

    void CSpotLight::OnSelectedPreFrameRender()
    {
        constexpr float Step       = glm::radians(15.f);
        const float     Radius     = AttenuationRadius / glm::tan(OuterCutOffRad);
        const glm::vec3 BaseMiddle = Transform.Translation + (Direction * AttenuationRadius);

        for (float Rotation = 0; Rotation < 6.14; Rotation += Step)
        {
            glm::vec3 LineEnd = BaseMiddle + ((glm::angleAxis(Rotation, Direction) * LightUp) * Radius);
            GEngine.GetRenderer()->AddDebugLine(Transform.Translation, LineEnd, Color, Color);
        }

        {
            const FDebugArrow DirectionArrow = MakeDebugArrowData(Transform.Translation, Direction, 2);
            GEngine.GetRenderer()->AddDebugLine(DirectionArrow.BodyStart, DirectionArrow.BodyEnd, Color, Color);
            GEngine.GetRenderer()->AddDebugLine(DirectionArrow.HeadStart0, DirectionArrow.HeadEnd0, Color, Color);
            GEngine.GetRenderer()->AddDebugLine(DirectionArrow.HeadStart1, DirectionArrow.HeadEnd1, Color, Color);
        }
    }

#endif

    IActor* CSpotLight::CreateActorCopy()
    {
        auto* Copy              = new CSpotLight{ Name.GetCopy(), Parent, World };
        Copy->Direction         = Direction;
        Copy->Color             = Color;
        Copy->LightUp           = LightUp;
        Copy->Quality           = Quality;
        Copy->AttenuationRadius = AttenuationRadius;
        Copy->InnerCutOffRad    = InnerCutOffRad;
        Copy->OuterCutOffRad    = OuterCutOffRad;
        Copy->Transform         = Transform;
        Copy->Transform.Translation += glm::vec3{ 1, 0, 0 };
        Copy->bShouldCastShadow = bShouldCastShadow;
        if (ShadowMap)
        {
            ShadowMap = GEngine.GetRenderer()->CreateShadowMap(ELightType::SPOT);
        }
        World->AddSpotLight(Copy);
        return Copy;
    }

    void CSpotLight::OnAddToWorld(CWorld* InWorld)
    {
        CLight::OnAddToWorld(InWorld);
        InWorld->AddSpotLight(this);
        if (bShouldCastShadow && !ShadowMap)
        {
            ShadowMap = GEngine.GetRenderer()->CreateShadowMap(ELightType::DIRECTIONAL);
        }
    }

    void CSpotLight::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        CLight::OnRemoveFromWorld(InbHardRemove);
        World->RemoveSpotLight(ActorId);
        if (InbHardRemove)
        {
            CleanupAfterRemove();
        }
    }

    /////////////////////////////////////
    //           Point light           //
    /////////////////////////////////////

    void CPointLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        const float ShadowMapWidth  = (float)ShadowMap->GetShadowMapTexture()->GetWidth();
        const float ShadowMapHeight = (float)ShadowMap->GetShadowMapTexture()->GetHeight();

        CachedNearPlane = LightSettings.Near;
        CachedFarPlane  = LightSettings.Far;

        const glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.f), ShadowMapWidth / ShadowMapHeight, CachedNearPlane, CachedFarPlane);

        LightSpaceMatrices[0] =
          projectionMatrix * glm::lookAt(Transform.Translation, Transform.Translation + glm::vec3{ 1.0, 0.0, 0.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
        LightSpaceMatrices[1] =
          projectionMatrix * glm::lookAt(Transform.Translation, Transform.Translation + glm::vec3{ -1.0, 0.0, 0.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
        LightSpaceMatrices[2] =
          projectionMatrix * glm::lookAt(Transform.Translation, Transform.Translation + glm::vec3{ 0.0, 1.0, 0.0 }, glm::vec3{ 0.0, 0.0, 1.0 });
        LightSpaceMatrices[3] =
          projectionMatrix * glm::lookAt(Transform.Translation, Transform.Translation + glm::vec3{ 0.0, -1.0, 0.0 }, glm::vec3{ 0.0, 0.0, -1.0 });
        LightSpaceMatrices[4] =
          projectionMatrix * glm::lookAt(Transform.Translation, Transform.Translation + glm::vec3{ 0.0, 0.0, 1.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
        LightSpaceMatrices[5] =
          projectionMatrix * glm::lookAt(Transform.Translation, Transform.Translation + glm::vec3{ 0.0, 0.0, -1.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
    }

    void CPointLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);
        InShader->SetFloat(ATTENUATION_RADIUS, AttenuationRadius);
        InShader->SetFloat(LIGHT_NEAR_PLANE, CachedNearPlane);
        InShader->SetFloat(LIGHT_FAR_PLANE, CachedFarPlane);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_0, LightSpaceMatrices[0]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_1, LightSpaceMatrices[1]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_2, LightSpaceMatrices[2]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_3, LightSpaceMatrices[3]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_4, LightSpaceMatrices[4]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_5, LightSpaceMatrices[5]);

        if (ShadowMap)
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
            InShader->UseTexture(LIGHT_SHADOW_CUBE, ShadowMap->GetShadowMapTexture());
        }
        else
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
        }
    }

    void CPointLight::SetupShadowMapShader(gpu::CShader* InShader)
    {
        InShader->SetVector(LIGHT_POSITION, Transform.Translation);
        InShader->SetFloat(LIGHT_FAR_PLANE, CachedFarPlane);
        InShader->SetVector(LIGHT_DIRECTION, Transform.Translation);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_0, LightSpaceMatrices[0]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_1, LightSpaceMatrices[1]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_2, LightSpaceMatrices[2]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_3, LightSpaceMatrices[3]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_4, LightSpaceMatrices[4]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_5, LightSpaceMatrices[5]);
    }

#if DEVELOPMENT
    void CPointLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();
        if (ImGui::CollapsingHeader("Spot light"))
        {
            ImGui::DragFloat("Attenuation Radius", &AttenuationRadius, 1, 0, 10000);
        }
    }

    void CPointLight::OnSelectedPreFrameRender()
    {
        DrawDebugSphere(Transform.Translation, AttenuationRadius, Color);
    }

#endif

    IActor* CPointLight::CreateActorCopy()
    {
        auto* Copy              = new CPointLight{ Name.GetCopy(), Parent, World };
        Copy->Color             = Color;
        Copy->Quality           = Quality;
        Copy->AttenuationRadius = AttenuationRadius;
        Copy->CachedNearPlane   = CachedNearPlane;
        Copy->CachedFarPlane    = CachedFarPlane;
        Copy->Transform         = Transform;
        Copy->Transform.Translation += glm::vec3{ 1, 0, 0 };
        Copy->bShouldCastShadow = bShouldCastShadow;
        if (ShadowMap)
        {
            ShadowMap = GEngine.GetRenderer()->CreateShadowMap(ELightType::POINT);
        }
        World->AddPointLight(Copy);
        return Copy;
    }

    void CPointLight::OnAddToWorld(CWorld* InWorld)
    {
        CLight::OnAddToWorld(InWorld);
        InWorld->AddPointLight(this);
        if (bShouldCastShadow && !ShadowMap)
        {
            ShadowMap = GEngine.GetRenderer()->CreateShadowMap(ELightType::DIRECTIONAL);
        }
    }

    void CPointLight::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        CLight::OnRemoveFromWorld(InbHardRemove);
        World->RemovePointLight(ActorId);
        if (InbHardRemove)
        {
            CleanupAfterRemove();
        }
    }

} // namespace lucid::scene
