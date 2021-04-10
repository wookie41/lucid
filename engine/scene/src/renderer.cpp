#include "scene/renderer.hpp"

#include <stb_ds.h>

#include "devices/gpu/cubemap.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid::scene
{
    /////////////////////////////////////
    //         Lights/ShadowMaps       //
    /////////////////////////////////////

    CDirectionalLight* CRenderer::CreateDirectionalLight(const FDString& InName, const IActor* InParent, const bool& CastsShadow)
    {
        auto* DirectionalLight = new CDirectionalLight(InName, InParent);

        if (CastsShadow)
        {
            DirectionalLight->ShadowMap = CreateShadowMap(ELightType::DIRECTIONAL);
        }

        arrput(CreatedLights, DirectionalLight);
        return DirectionalLight;
    }

    CSpotLight* CRenderer::CreateSpotLight(const FDString& InName, const IActor* InParent, const bool& CastsShadow)
    {
        auto* SpotLight = new CSpotLight(InName, InParent);

        if (CastsShadow)
        {
            SpotLight->ShadowMap = CreateShadowMap(ELightType::SPOT);
        }

        arrput(CreatedLights, SpotLight);
        return SpotLight;
    }

    CPointLight* CRenderer::CreatePointLight(const FDString& InName, const IActor* InParent, const bool& CastsShadow)
    {
        auto* PointLight = new CPointLight(InName, InParent);

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
            gpu::CTexture* ShadowMapTexture = gpu::CreateCubemap(ShadowMapSizeByQuality[DefaultShadowMapQuality],
                                                                 gpu::ETextureDataFormat::DEPTH_COMPONENT,
                                                                 gpu::ETexturePixelFormat::DEPTH_COMPONENT,
                                                                 gpu::ETextureDataType::FLOAT,
                                                                 nullptr,
                                                                 FSString{ "ShadowCubemap" });

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
        ShadowMapTexture->SetWrapSFilter(lucid::gpu::WrapTextureFilter::CLAMP_TO_EDGE);
        ShadowMapTexture->SetWrapTFilter(lucid::gpu::WrapTextureFilter::CLAMP_TO_EDGE);
        ShadowMapTexture->SetMinFilter(lucid::gpu::MinTextureFilter::NEAREST);
        ShadowMapTexture->SetMagFilter(lucid::gpu::MagTextureFilter::NEAREST);

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
} // namespace lucid::scene
