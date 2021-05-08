#include "scene/renderer.hpp"

#include <stb_ds.h>
#include <devices/gpu/texture_enums.hpp>


#include "devices/gpu/cubemap.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid::scene
{
    /////////////////////////////////////
    //         Lights/ShadowMaps       //
    /////////////////////////////////////

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
                                                                 gpu::EWrapTextureFilter::CLAMP_TO_BORDER,
                                                                 gpu::EWrapTextureFilter::CLAMP_TO_BORDER,
                                                                 gpu::EWrapTextureFilter::CLAMP_TO_BORDER,
                                                                 {1, 1, 1, 1});

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
        ShadowMapTexture->SetBorderColor({1, 1, 1, 1});

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
