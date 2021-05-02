#include "scene/actors/lights.hpp"

#include <engine/engine.hpp>


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
    static const FSString LIGHT_CONSTANT("uLightConstant");
    static const FSString LIGHT_LINEAR("uLightLinear");
    static const FSString LIGHT_QUADRATIC("uLightQuadratic");
    static const FSString LIGHT_INNER_CUT_OFF("uLightInnerCutOffCos");
    static const FSString LIGHT_OUTER_CUT_OFF("uLightOuterCutOffCos");
    static const FSString LIGHT_SHADOW_MAP("uLightShadowMap");
    static const FSString LIGHT_CASTS_SHADOWS("uLightCastsShadows");
    static const FSString LIGHT_SHADOW_CUBE("uLightShadowCube");
    
    inline glm::mat4 CreateLightSpaceMatrix(const glm::vec3& Position, const glm::vec3& LightUp, const LightSettings& MatrixSettings)
    {
        const glm::mat4 viewMatrix = glm::lookAt(Position, glm::vec3{ 0 }, LightUp);
        const glm::mat4 projectionMatrix = glm::ortho(MatrixSettings.Left,
                                                      MatrixSettings.Right,
                                                      MatrixSettings.Bottom,
                                                      MatrixSettings.Top,
                                                      MatrixSettings.Near,
                                                      MatrixSettings.Far);
        return projectionMatrix * viewMatrix;
    }

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
        ImGui::SliderFloat3("Light color", &Color.r, 0, 1);

        bool bCastsShadow = ShadowMap != nullptr;
        ImGui::Checkbox("Casts shadow", &bCastsShadow);
        if (bCastsShadow)
        {
            if (!ShadowMap)
            {
                ShadowMap = GEngine.GetRenderer()->CreateShadowMap(GetType());
            }
        }
        else
        {
            if (ShadowMap)
            {
                ShadowMap->Free();
                ShadowMap = nullptr;
            }
        }
    }
#endif

    float CLight::GetVerticalMidPoint() const
    {
        return 0;
    }

    void CLight::_SaveToResourceFile(const FString& InFilePath)
    {
        // Light data is written directly to the world file
    }
    
    /////////////////////////////////////
    //        Directional light        //
    /////////////////////////////////////

    void CDirectionalLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        LightSpaceMatrix = CreateLightSpaceMatrix(Transform.Translation, LightUp, LightSettings);
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

    void CDirectionalLight::SetupShadowMapShader(gpu::CShader* InShader)
    {
        InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);
    }

#if DEVELOPMENT
    void CDirectionalLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();
        ImGui::SliderFloat3("Direction", &Direction.x, -1, 1);
        ImGui::SliderFloat3("Light up", &LightUp.x, 0, 1);
    }
#endif
    
    /////////////////////////////////////
    //            Spot light           //
    /////////////////////////////////////

    void CSpotLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        LightSpaceMatrix = CreateLightSpaceMatrix(Transform.Translation, LightUp, LightSettings);
    }

    void CSpotLight::SetupShader(gpu::CShader* InShader) const
    {
        CLight::SetupShader(InShader);
        InShader->SetVector(LIGHT_DIRECTION, Direction);
        InShader->SetFloat(LIGHT_CONSTANT, Constant);
        InShader->SetFloat(LIGHT_LINEAR, Linear);
        InShader->SetFloat(LIGHT_QUADRATIC, Quadratic);
        InShader->SetFloat(LIGHT_INNER_CUT_OFF, glm::cos(InnerCutOffRad));
        InShader->SetFloat(LIGHT_OUTER_CUT_OFF, glm::cos(OuterCutOffRad));
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

    void CSpotLight::SetupShadowMapShader(gpu::CShader* InShader)
    {
        InShader->SetMatrix(LIGHT_SPACE_MATRIX, LightSpaceMatrix);
    }

#if DEVELOPMENT
    void CSpotLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();
        ImGui::SliderFloat3("Direction", &Direction.x, -1, 1);
        ImGui::SliderFloat3("Light up", &LightUp.x, 0, 1);

        ImGui::SliderFloat("Constant", &Constant, 0, FLT_HALF_MAX);
        ImGui::SliderFloat("Linear", &Linear, 0, FLT_HALF_MAX);
        ImGui::SliderFloat("Quadratic", &Quadratic, 0, FLT_HALF_MAX);
        ImGui::SliderFloat("InnerCutOffRad", &InnerCutOffRad, 0, 2);
        ImGui::SliderFloat("OuterCutOffRad", &OuterCutOffRad, 0, 2);
    }
#endif
    
    /////////////////////////////////////
    //           Point light           //
    /////////////////////////////////////
    
    void CPointLight::UpdateLightSpaceMatrix(const LightSettings& LightSettings)
    {
        const float ShadowMapWidth = (float)ShadowMap->GetShadowMapTexture()->GetWidth();
        const float ShadowMapHeight = (float)ShadowMap->GetShadowMapTexture()->GetHeight();
        
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.f), ShadowMapWidth / ShadowMapHeight, NearPlane, FarPlane);

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
        InShader->SetFloat(LIGHT_CONSTANT, Constant);
        InShader->SetFloat(LIGHT_LINEAR, Linear);
        InShader->SetFloat(LIGHT_QUADRATIC, Quadratic);
        InShader->SetFloat(LIGHT_NEAR_PLANE, NearPlane);
        InShader->SetFloat(LIGHT_FAR_PLANE, FarPlane);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_0, LightSpaceMatrices[0]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_1, LightSpaceMatrices[1]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_2, LightSpaceMatrices[2]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_3, LightSpaceMatrices[3]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_4, LightSpaceMatrices[4]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_5, LightSpaceMatrices[5]);

        if (ShadowMap != nullptr)
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
        InShader->SetFloat(LIGHT_FAR_PLANE, FarPlane);
        InShader->SetVector(LIGHT_POSITION, Transform.Translation);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_1, LightSpaceMatrices[1]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_2, LightSpaceMatrices[2]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_3, LightSpaceMatrices[3]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_4, LightSpaceMatrices[4]);
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_5, LightSpaceMatrices[5]);        
        InShader->SetMatrix(LIGHT_SPACE_MATRIX_5, LightSpaceMatrices[5]);        
    }

    
#if DEVELOPMENT
    void CPointLight::UIDrawActorDetails()
    {
        CLight::UIDrawActorDetails();

        ImGui::SliderFloat("Constant", &Constant, 0, FLT_HALF_MAX);
        ImGui::SliderFloat("Linear", &Linear, 0, FLT_HALF_MAX);
        ImGui::SliderFloat("Quadratic", &Quadratic, 0, FLT_HALF_MAX);
        ImGui::SliderFloat("Near plane", &NearPlane, 1, 5);
        ImGui::SliderFloat("Far plane", &FarPlane, 5, 100);
    }
#endif
    
} // namespace lucid::scene