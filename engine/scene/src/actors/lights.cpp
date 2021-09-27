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
    static const FSString INV_ATTENUATION_RADIUS_SQUARED("uLightInvAttenuationRadiusSquared");

    static const FSString LIGHT_INNER_CUT_OFF("uLightInnerCutOffCos");
    static const FSString LIGHT_OUTER_CUT_OFF("uLightOuterCutOffCos");
    static const FSString LIGHT_SHADOW_MAP("uLightShadowMap");
    static const FSString LIGHT_CASTS_SHADOWS("uLightCastsShadows");
    static const FSString LIGHT_SHADOW_CUBE("uLightShadowCube");
    static const FSString LIGHT_INTENSITY("uLightIntensity");

    static const FSString CASCADE_COUNT("uCascadeCount");

    static const float LightEfficiencyByLightSourceType[]{
        3.5f, // Incandescent
        9.f, // LED
        12.5f // Fluorescent
    };

    void SetupLightIntensityBasedOnUnit(const ELightSourceType& LightSourceType,
                                        const ELightUnit&       LightUnit,
                                        const float&            LuminousPower,
                                        const float&            RadiantPower,
                                        gpu::CShader*           Shader)
    {
        float LightIntensity = 0;
        switch (LightUnit)
        {
        case ELightUnit::WATTS:
            LightIntensity = RadiantPower * 683.f * LightEfficiencyByLightSourceType[static_cast<u8>(LightSourceType)] / 4 * math::PI_F;
            break;

        case ELightUnit::LUMENS:
            LightIntensity = LuminousPower / 4 * math::PI_F;
            break;
        default:
            assert(0);
        }

        Shader->SetFloat(LIGHT_INTENSITY, LightIntensity);
    }

#if DEVELOPMENT
    void UIDrawLightIntensityPanel(float* LuminousPower, float* RadiantPower, ELightUnit* LightUnitType, ELightSourceType* LightSourceType)
    {
        ImGui::Text("Light units");
        if (ImGui::RadioButton("Lumens", *LightUnitType == ELightUnit::LUMENS))
        {
            *LightUnitType = ELightUnit::LUMENS;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Watts", *LightUnitType == ELightUnit::WATTS))
        {
            *LightUnitType = ELightUnit::WATTS;
        }

        if (*LightUnitType == ELightUnit::WATTS)
        {
            ImGui::Text("Light source type");

            if (ImGui::RadioButton("Incandescent", *LightSourceType == ELightSourceType::INCANDESCENT))
            {
                *LightSourceType = ELightSourceType::INCANDESCENT;
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("LED", *LightSourceType == ELightSourceType::LED))
            {
                *LightSourceType = ELightSourceType::LED;
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("Fluorescent", *LightSourceType == ELightSourceType::FLUORESCENT))
            {
                *LightSourceType = ELightSourceType::FLUORESCENT;
            }

            ImGui::DragFloat("Radiant power (W)", RadiantPower, 1, 0, 10000);
        }
        else
        {
            ImGui::DragFloat("Luminous power (lm)", LuminousPower, 1, 0, 1000000);
        }
    }
#endif

    void CLight::SetupShader(gpu::CShader* InShader) const
    {
        InShader->SetInt(LIGHT_TYPE, static_cast<u32>(GetType()));
        InShader->SetVector(LIGHT_POSITION, GetTransform().Translation);
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

            bool bWasCastingShadow = bCastsShadow;
            if (ImGui::Checkbox("Casts shadow", &bCastsShadow))
            {
                if (!bWasCastingShadow)
                {
                    CreateShadowMap();
                }
                else if (ShadowMap)
                {
                    FreeShadowMap();
                }
            }
        }
    }
#endif

    void CLight::CreateShadowMap() { ShadowMap = GEngine.GetRenderer()->CreateShadowMap(GetType()); }

    void CLight::FreeShadowMap()
    {
        ShadowMap->Free();
        ShadowMap = nullptr;
    }

    void CLight::InternalSaveAssetToFile(const FString& InFilePath)
    {
        // Light data is written directly to the world file
    }

    void CLight::CleanupAfterRemove()
    {
        if (bCastsShadow)
        {
            FreeShadowMap();
        }
    }

    /////////////////////////////////////
    //        Directional light        //
    /////////////////////////////////////

    void CDirectionalLight::CreateShadowMap()
    {
        for (u8 i = 0; i < CascadeCount; ++i)
        {
            CascadeShadowMaps[i] = GEngine.GetRenderer()->CreateShadowMap(ELightType::DIRECTIONAL);
        }
    }
    void CDirectionalLight::FreeShadowMap()
    {
        for (u8 i = 0; i < CascadeCount; ++i)
        {
            CascadeShadowMaps[i]->Free();
            CascadeShadowMaps[i] = nullptr;
        }
    }

    void CDirectionalLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        // noop, dir lights are using CSMs
    }

    void CDirectionalLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);

        InShader->SetVector(LIGHT_DIRECTION, Direction);
        InShader->SetFloat(LIGHT_INTENSITY, Illuminance);

        if (bCastsShadow)
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, true);
            InShader->SetInt(CASCADE_COUNT, CascadeCount);

            for (int i = 0; i < CascadeCount; ++i)
            {
                std::ostringstream UniformParamName;
                UniformParamName << "uCascadeMatrices[" << i << "]";

                const std::string CascadeMatrixName = UniformParamName.str();

                UniformParamName.str("");
                UniformParamName << "uCascadeFarPlanes[" << i << "]";

                const std::string CascadeFarPlaneName = UniformParamName.str();

                UniformParamName.str("");
                UniformParamName << "uCascadeShadowMaps[" << i << "]";

                const std::string CascadeShadowMapName = UniformParamName.str();

                InShader->SetMatrix((char*)CascadeMatrixName.c_str(), CascadeMatrices[i]);
                InShader->SetFloat((char*)CascadeFarPlaneName.c_str(), CascadeFarPlanes[i]);
                InShader->UseBindlessTexture((char*)CascadeShadowMapName.c_str(), CascadeShadowMaps[i]->GetShadowMapTexture()->GetBindlessHandle());
            }
        }
        else
        {
            InShader->SetBool(LIGHT_CASTS_SHADOWS, false);
        }
    }

    void CDirectionalLight::SetupShadowMapShader(gpu::CShader* InShader) { assert(0); }

#if DEVELOPMENT
    void CDirectionalLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();
        if (ImGui::CollapsingHeader("Directional light"))
        {
            int NewCascadeCount = CascadeCount;

            ImGui::DragFloat3("Light up", &LightUp.x, 0.001, -1, 1);
            ImGui::DragFloat("Illuminance (lux)", &Illuminance, 1);
            ImGui::InputInt("Number of cascades", (int*)&NewCascadeCount, 1, 1);
            ImGui::DragFloat("Cascade split log factor", &CascadeSplitLogFactor, 0.01, 0.1, 1);
            ImGui::DragFloat("First cascade near plane", &FirstCascadeNearPlane, 0.01, 0.1, 10);
            ImGui::DragFloat("Shadows max distance", &ShadowsMaxDistance, 0.01, 1, 10000);
            ImGui::DragFloat("Shadows Z multiplier", &ShadowsZMuliplier, 0.01, 0.1, 100);

            if (NewCascadeCount != CascadeCount && NewCascadeCount <= MAX_SHADOW_CASCADES)
            {
                while (NewCascadeCount > CascadeCount)
                {
                    CascadeShadowMaps[CascadeCount] = GEngine.GetRenderer()->CreateShadowMap(ELightType::DIRECTIONAL);
                    CascadeShadowMaps[CascadeCount]->GetShadowMapTexture()->GetBindlessHandle();
                    CascadeShadowMaps[CascadeCount]->GetShadowMapTexture()->MakeBindlessResident();
                    ++CascadeCount;
                }
                
                while (NewCascadeCount < CascadeCount)
                {
                    CascadeShadowMaps[CascadeCount - 1]->Free();
                    CascadeShadowMaps[CascadeCount - 1] = nullptr;
                    --CascadeCount;
                }
            }
        }
    }

    void CDirectionalLight::OnSelectedPreFrameRender()
    {
        CLight::OnSelectedPreFrameRender();

        constexpr glm::vec3 ArrowsOffsets[] = {
            { 0, 0, 0 },
            { 0, 2, 0 },
            { 0, -3, 0 },
        };

        const FDebugArrow DebugArrow = MakeDebugArrowData(GetTransform().Translation, Direction, 20);

        for (int i = 0; i < sizeof(ArrowsOffsets) / sizeof(glm::vec3); ++i)
        {
            GEngine.GetRenderer()->DrawDebugLine(DebugArrow.BodyStart + ArrowsOffsets[i], DebugArrow.BodyEnd + ArrowsOffsets[i], Color, Color);
            GEngine.GetRenderer()->DrawDebugLine(DebugArrow.HeadStart0 + ArrowsOffsets[i], DebugArrow.HeadEnd0 + ArrowsOffsets[i], Color, Color);
            GEngine.GetRenderer()->DrawDebugLine(DebugArrow.HeadStart1 + ArrowsOffsets[i], DebugArrow.HeadEnd1 + ArrowsOffsets[i], Color, Color);
        }
    }
#endif

    IActor* CDirectionalLight::CreateActorCopy()
    {
        auto* Copy         = new CDirectionalLight{ Name.GetCopy(), Parent, World };
        Copy->Direction    = Direction;
        Copy->Color        = Color;
        Copy->LightUp      = LightUp;
        Copy->Quality      = Quality;
        Copy->bCastsShadow = bCastsShadow;
        Copy->Illuminance  = Illuminance;
        Copy->SetTransform(GetTransform());
        Copy->Translate({ 1, 0, 0 });

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
        if (bCastsShadow && !ShadowMap)
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

        const glm::mat4 ViewMatrix       = glm::lookAt(GetTransform().Translation, GetTransform().Translation + Direction, LightUp);
        const glm::mat4 ProjectionMatrix = glm::perspective(OuterCutOffRad * 2, ShadowMapWidth / ShadowMapHeight, LightSettings.Near, LightSettings.Far);
        LightSpaceMatrix                 = ProjectionMatrix * ViewMatrix;
    }

    void CSpotLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);
        InShader->SetVector(LIGHT_DIRECTION, Direction);
        InShader->SetFloat(ATTENUATION_RADIUS, AttenuationRadius);
        InShader->SetFloat(INV_ATTENUATION_RADIUS_SQUARED, powf(1.f / AttenuationRadius, 2.f));
        InShader->SetFloat(LIGHT_INNER_CUT_OFF, glm::cos(InnerCutOffRad));
        InShader->SetFloat(LIGHT_OUTER_CUT_OFF, glm::cos(OuterCutOffRad));
        InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);

        SetupLightIntensityBasedOnUnit(LightSourceType, LightUnit, LuminousPower, RadiantPower, InShader);

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

            UIDrawLightIntensityPanel(&LuminousPower, &RadiantPower, &LightUnit, &LightSourceType);
        }
    }

    void CSpotLight::OnSelectedPreFrameRender()
    {
        CLight::OnSelectedPreFrameRender();

        constexpr float Step       = glm::radians(15.f);
        const float     Radius     = AttenuationRadius / glm::tan(OuterCutOffRad);
        const glm::vec3 BaseMiddle = GetTransform().Translation + (Direction * AttenuationRadius);

        for (float Rotation = 0; Rotation < 6.14; Rotation += Step)
        {
            glm::vec3 LineEnd = BaseMiddle + ((glm::angleAxis(Rotation, Direction) * LightUp) * Radius);
            GEngine.GetRenderer()->DrawDebugLine(GetTransform().Translation, LineEnd, Color, Color);
        }

        {
            const FDebugArrow DirectionArrow = MakeDebugArrowData(GetTransform().Translation, Direction, 2);
            GEngine.GetRenderer()->DrawDebugLine(DirectionArrow.BodyStart, DirectionArrow.BodyEnd, Color, Color);
            GEngine.GetRenderer()->DrawDebugLine(DirectionArrow.HeadStart0, DirectionArrow.HeadEnd0, Color, Color);
            GEngine.GetRenderer()->DrawDebugLine(DirectionArrow.HeadStart1, DirectionArrow.HeadEnd1, Color, Color);
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
        Copy->bCastsShadow      = bCastsShadow;
        Copy->LightUnit         = LightUnit;
        Copy->LightSourceType   = LightSourceType;
        Copy->RadiantPower      = RadiantPower;
        Copy->LuminousPower     = LuminousPower;
        Copy->SetTransform(GetTransform());
        Copy->Translate({ 1, 0, 0 });

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
        if (bCastsShadow && !ShadowMap)
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

    void CSpotLight::OnRotated(const glm::quat& in_old_rotation, const glm::quat& in_new_rotation)
    {
        CLight::OnRotated(in_old_rotation, in_new_rotation);

        Direction = glm::normalize(glm::vec3{ 0, 0, 1 } * GetTransform().Rotation);
        LightUp   = glm::normalize(glm::vec3{ 0, 1, 0 } * GetTransform().Rotation);
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
          projectionMatrix * glm::lookAt(GetTransform().Translation, GetTransform().Translation + glm::vec3{ 1.0, 0.0, 0.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
        LightSpaceMatrices[1] =
          projectionMatrix * glm::lookAt(GetTransform().Translation, GetTransform().Translation + glm::vec3{ -1.0, 0.0, 0.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
        LightSpaceMatrices[2] =
          projectionMatrix * glm::lookAt(GetTransform().Translation, GetTransform().Translation + glm::vec3{ 0.0, 1.0, 0.0 }, glm::vec3{ 0.0, 0.0, 1.0 });
        LightSpaceMatrices[3] =
          projectionMatrix * glm::lookAt(GetTransform().Translation, GetTransform().Translation + glm::vec3{ 0.0, -1.0, 0.0 }, glm::vec3{ 0.0, 0.0, -1.0 });
        LightSpaceMatrices[4] =
          projectionMatrix * glm::lookAt(GetTransform().Translation, GetTransform().Translation + glm::vec3{ 0.0, 0.0, 1.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
        LightSpaceMatrices[5] =
          projectionMatrix * glm::lookAt(GetTransform().Translation, GetTransform().Translation + glm::vec3{ 0.0, 0.0, -1.0 }, glm::vec3{ 0.0, -1.0, 0.0 });
    }

    void CPointLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);
        InShader->SetFloat(ATTENUATION_RADIUS, AttenuationRadius);
        InShader->SetFloat(INV_ATTENUATION_RADIUS_SQUARED, powf(1.f / AttenuationRadius, 2.f));
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

        SetupLightIntensityBasedOnUnit(LightSourceType, LightUnit, LuminousPower, RadiantPower, InShader);
    }

    void CPointLight::SetupShadowMapShader(gpu::CShader* InShader)
    {
        InShader->SetVector(LIGHT_POSITION, GetTransform().Translation);
        InShader->SetFloat(LIGHT_FAR_PLANE, CachedFarPlane);
        InShader->SetVector(LIGHT_DIRECTION, GetTransform().Translation);
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
        if (ImGui::CollapsingHeader("Point light"))
        {
            ImGui::DragFloat("Attenuation Radius", &AttenuationRadius, 1, 0, 10000);

            UIDrawLightIntensityPanel(&LuminousPower, &RadiantPower, &LightUnit, &LightSourceType);
        }
    }

    void CPointLight::OnSelectedPreFrameRender() { DrawDebugSphere(GetTransform().Translation, AttenuationRadius, Color); }

#endif

    IActor* CPointLight::CreateActorCopy()
    {
        auto* Copy              = new CPointLight{ Name.GetCopy(), Parent, World };
        Copy->Color             = Color;
        Copy->Quality           = Quality;
        Copy->AttenuationRadius = AttenuationRadius;
        Copy->CachedNearPlane   = CachedNearPlane;
        Copy->CachedFarPlane    = CachedFarPlane;
        Copy->bCastsShadow      = bCastsShadow;
        Copy->LightUnit         = LightUnit;
        Copy->LightSourceType   = LightSourceType;
        Copy->RadiantPower      = RadiantPower;
        Copy->LuminousPower     = LuminousPower;
        Copy->SetTransform(GetTransform());
        Copy->Translate({ 1, 0, 0 });

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
        if (bCastsShadow && !ShadowMap)
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
