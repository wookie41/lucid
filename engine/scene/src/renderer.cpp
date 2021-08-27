#include "scene/renderer.hpp"

#include <stb_ds.h>

#include "engine/engine.hpp"

#include "platform/util.hpp"

#include "devices/gpu/texture_enums.hpp"
#include "devices/gpu/cubemap.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid::scene
{
    /////////////////////////////////////
    //         Lights/ShadowMaps       //
    /////////////////////////////////////

    FRenderStats GRenderStats;

    CDirectionalLight* CRenderer::CreateDirectionalLight(const FDString& InName, IActor* InParent, CWorld* InWorld, const bool& CastsShadow)
    {
        auto* DirectionalLight = new CDirectionalLight(InName, InParent, InWorld);

        if (CastsShadow)
        {
            DirectionalLight->ShadowMap = CreateShadowMap(ELightType::DIRECTIONAL);
        }

        arrput(CreatedLights, DirectionalLight);
        return DirectionalLight;
    }

    CSpotLight* CRenderer::CreateSpotLight(const FDString& InName, IActor* InParent, CWorld* InWorld, const bool& CastsShadow)
    {
        auto* SpotLight = new CSpotLight(InName, InParent, InWorld);

        if (CastsShadow)
        {
            SpotLight->ShadowMap = CreateShadowMap(ELightType::SPOT);
        }

        arrput(CreatedLights, SpotLight);
        return SpotLight;
    }

    CPointLight* CRenderer::CreatePointLight(const FDString& InName, IActor* InParent, CWorld* InWorld, const bool& CastsShadow)
    {
        auto* PointLight = new CPointLight(InName, InParent, InWorld);

        if (CastsShadow)
        {
            PointLight->ShadowMap = CreateShadowMap(ELightType::POINT);
        }

        arrput(CreatedLights, PointLight);
        return PointLight;
    }

    CShadowMap* CRenderer::CreateShadowMap(const ELightType& InLightType)
    {
        if (InLightType == ELightType::POINT)
        {
            gpu::CTexture* ShadowMapTexture = gpu::CreateCubemap(ShadowMapSizeByQuality[DefaultShadowMapQuality].x,
                                                                 ShadowMapSizeByQuality[DefaultShadowMapQuality].y,
                                                                 gpu::ETextureDataFormat::DEPTH_COMPONENT,
                                                                 gpu::ETexturePixelFormat::DEPTH_COMPONENT,
                                                                 gpu::ETextureDataType::FLOAT,
                                                                 nullptr,
                                                                 FSString{ "ShadowCubemap" },
                                                                 gpu::EMinTextureFilter::LINEAR,
                                                                 gpu::EMagTextureFilter::LINEAR,
                                                                 gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                                                 gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                                                 gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                                                 { 0, 0, 0, 0 });

            auto* ShadowMap = new CShadowMap(arrlen(CreatedShadowMaps), ShadowMapTexture, DefaultShadowMapQuality);
            arrput(CreatedShadowMaps, ShadowMap);
            return ShadowMap;
        }
        gpu::CTexture* ShadowMapTexture = gpu::CreateEmpty2DTexture(ShadowMapSizeByQuality[DefaultShadowMapQuality].x,
                                                                    ShadowMapSizeByQuality[DefaultShadowMapQuality].y,
                                                                    gpu::ETextureDataType::FLOAT,
                                                                    gpu::ETextureDataFormat::DEPTH_COMPONENT,
                                                                    gpu::ETexturePixelFormat::DEPTH_COMPONENT,
                                                                    0,
                                                                    FSString{ "ShadowMap" });

        ShadowMapTexture->Bind();
        ShadowMapTexture->SetWrapSFilter(lucid::gpu::EWrapTextureFilter::CLAMP_TO_BORDER);
        ShadowMapTexture->SetWrapTFilter(lucid::gpu::EWrapTextureFilter::CLAMP_TO_BORDER);
        ShadowMapTexture->SetMinFilter(lucid::gpu::EMinTextureFilter::NEAREST);
        ShadowMapTexture->SetMagFilter(lucid::gpu::EMagTextureFilter::NEAREST);
        ShadowMapTexture->SetBorderColor({ 0, 0, 0, 1 });

        auto* ShadowMap = new CShadowMap(arrlen(CreatedShadowMaps), ShadowMapTexture, DefaultShadowMapQuality);
        arrput(CreatedShadowMaps, ShadowMap);
        return ShadowMap;
    }

    CShadowMap::CShadowMap(const RID& InId, gpu::CTexture* InShadowMapTexture, const u8& InShadowMapQuality)
    : CRendererObject(InId), ShadowMapTexture(InShadowMapTexture), ShadowMapQuality(InShadowMapQuality)
    {
        ShadowCubeMapTexture = dynamic_cast<gpu::CCubemap*>(InShadowMapTexture);
        assert(ShadowCubeMapTexture);
    }

    void CShadowMap::Free()
    {
        if (ShadowMapTexture)
        {
            ShadowMapTexture->Free();
            delete ShadowMapTexture;
            ShadowMapTexture = nullptr;
        }
    }

    void CRenderer::RemoveShadowMap(CShadowMap* InShadowMap)
    {
        InShadowMap->Free();
        delete InShadowMap;
    }

#if DEVELOPMENT
    void CRenderer::AddDebugLine(const glm::vec3&  InStart,
                                 const glm::vec3&  InEnd,
                                 const glm::vec3&  InStartColor,
                                 const glm::vec3&  InEndColor,
                                 const float&      InPersistTime,
                                 const ESpaceType& InSpaceType)
    {
        DebugLines.emplace_back(
          InStart, InEnd, InStartColor, InEndColor, InPersistTime < 0 ? -1 : platform::GetCurrentTimeSeconds() + InPersistTime, InSpaceType);
    }

    void CRenderer::RemoveStaleDebugLines()
    {
        const float             CurrentTime = platform::GetCurrentTimeSeconds();
        std::vector<FDebugLine> PersistedLines;

        for (int i = 0; i < DebugLines.size(); ++i)
        {
            const FDebugLine& DebugLine = DebugLines[i];
            if (DebugLine.RemoveTime < 0 || DebugLine.RemoveTime > CurrentTime)
            {
                PersistedLines.push_back(DebugLine);
            }
        }

        DebugLines.clear();
        DebugLines = std::move(PersistedLines);
    }

    FDebugArrow MakeDebugArrowData(const glm::vec3& InStart, const glm::vec3& InDirection, const float& InLength)
    {
        FDebugArrow DebugArrow;

        DebugArrow.BodyStart = InStart;
        DebugArrow.BodyEnd   = InStart + (InDirection * InLength);

        constexpr glm::vec3 HeadRise{ 0, 0.15, 0 };

        DebugArrow.HeadStart0 = DebugArrow.BodyEnd;
        DebugArrow.HeadEnd0   = DebugArrow.HeadStart0 + (((-InDirection) + HeadRise) * InLength / 10.f);

        DebugArrow.HeadStart1 = DebugArrow.BodyEnd;
        DebugArrow.HeadEnd1   = DebugArrow.HeadStart1 + (((-InDirection) - HeadRise) * InLength / 10.f);

        return DebugArrow;
    }

#endif

    void DrawDebugSphere(const glm::vec3& InCenter, const float& InRadius, const glm::vec3& InColor)
    {
        const float     SphereRadius = InRadius;
        const glm::vec3 SphereCenter = InCenter;
        constexpr float RotationStep = glm::radians(15.f);

        for (float Rotation = 0; Rotation < 6.14; Rotation += RotationStep)
        {
            const float CurrentPointOffsetSin = glm::sin(Rotation) * SphereRadius;
            const float CurrentPointOffsetCos = glm::cos(Rotation) * SphereRadius;

            const float NextPointOffsetSin = glm::sin(Rotation + RotationStep) * SphereRadius;
            const float NextPointOffsetCos = glm::cos(Rotation + RotationStep) * SphereRadius;

            const glm::vec3 AroundYStart = SphereCenter + glm::vec3{ CurrentPointOffsetSin, 0, CurrentPointOffsetCos };
            const glm::vec3 AroundYEnd   = SphereCenter + glm::vec3{ NextPointOffsetSin, 0, NextPointOffsetCos };

            GEngine.GetRenderer()->AddDebugLine(AroundYStart, AroundYEnd, InColor, InColor);

            const glm::vec3 AroundXStart = SphereCenter + glm::vec3{ 0, CurrentPointOffsetSin, CurrentPointOffsetCos };
            const glm::vec3 AroundXEnd   = SphereCenter + glm::vec3{ 0, NextPointOffsetSin, NextPointOffsetCos };

            GEngine.GetRenderer()->AddDebugLine(AroundXStart, AroundXEnd, InColor, InColor);

            const glm::vec3 AroundZStart = SphereCenter + glm::vec3{ CurrentPointOffsetCos, CurrentPointOffsetSin, 0 };
            const glm::vec3 AroundZEnd   = SphereCenter + glm::vec3{ NextPointOffsetCos, NextPointOffsetSin, 0 };

            GEngine.GetRenderer()->AddDebugLine(AroundZStart, AroundZEnd, InColor, InColor);
        }
    }

} // namespace lucid::scene
