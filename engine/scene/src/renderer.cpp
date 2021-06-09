﻿#include "scene/renderer.hpp"

#include <stb_ds.h>

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
        ShadowMapTexture->SetBorderColor({ 1, 1, 1, 1 });

        auto* ShadowMap = new CShadowMap(arrlen(CreatedShadowMaps), ShadowMapTexture, DefaultShadowMapQuality);
        arrput(CreatedShadowMaps, ShadowMap);
        return ShadowMap;
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
        DebugLines.emplace_back(InStart, InEnd, InStartColor, InEndColor, InPersistTime < 0 ? -1 : platform::GetCurrentTimeSeconds() + InPersistTime, InSpaceType);
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

#endif

} // namespace lucid::scene
