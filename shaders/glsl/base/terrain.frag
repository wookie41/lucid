#version 450 core

#include "common.glsl"
#include "terrain_uniforms.glsl"
#include "batch_instance.glsl"

flat in int InstanceID;

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
}
fsIn;

#include "lights.glsl"

out vec4 oFragColor;

#include "shadow_mapping.glsl"

void main()
{
    vec3 Normal  = normalize(fsIn.InterpolatedNormal);
    vec3 ToViewN = normalize(uViewPos - fsIn.FragPos);

    float ShadowFactor = 1.0;

    int LayerIndex = -1;

    for (int i = 0; i < MATERIAL_DATA.NumLayers; ++i)
    {
        if (fsIn.FragPos.y <= MATERIAL_DATA.Layers[i].MaxHeight)
        {
            LayerIndex = i;
            break;
        }
    }
    
    if (LayerIndex == -1 && MATERIAL_DATA.NumLayers > 0)
    {
        LayerIndex = MATERIAL_DATA.NumLayers - 1;
    }
    
    vec3 TerrainDiffuse = vec3(1);
    
    if (LayerIndex > -1)
    {
        vec2 LayerTextureCoords = fsIn.TextureCoords * MATERIAL_DATA.Layers[LayerIndex].UVTiling;

        if (MATERIAL_DATA.Layers[LayerIndex].bHasDiffuseMap)
        {
            sampler2D LayerDiffuseMap = MATERIAL_DATA.Layers[LayerIndex].DiffuseMap;
            TerrainDiffuse            = vec3(texture(LayerDiffuseMap, LayerTextureCoords));
        }

        if (MATERIAL_DATA.Layers[LayerIndex].bHasNormalMap)
        {
            sampler2D LayerNormalMap = MATERIAL_DATA.Layers[LayerIndex].NormalMap;
            Normal                   = texture(LayerNormalMap, LayerTextureCoords).rgb * 2 - 1;
        }

        if (LayerIndex - 1 > -1)
        {
            vec2 PrevLayerTextureCoords = fsIn.TextureCoords * MATERIAL_DATA.Layers[LayerIndex - 1].UVTiling;
            float LayersHeightDiff = MATERIAL_DATA.Layers[LayerIndex].MaxHeight - MATERIAL_DATA.Layers[LayerIndex - 1].MaxHeight;
            float BlendFactor = (MATERIAL_DATA.Layers[LayerIndex].MaxHeight - fsIn.FragPos.y) / LayersHeightDiff;

            if (MATERIAL_DATA.Layers[LayerIndex - 1].bHasDiffuseMap)
            {
                sampler2D NextLayerDiffuseMap = MATERIAL_DATA.Layers[LayerIndex - 1].DiffuseMap;
                TerrainDiffuse                = mix(TerrainDiffuse, texture(NextLayerDiffuseMap, PrevLayerTextureCoords).rgb, BlendFactor);
            }

            if (MATERIAL_DATA.Layers[LayerIndex - 1].bHasNormalMap)
            {
                sampler2D PrevLayerNormalMap = MATERIAL_DATA.Layers[LayerIndex - 1].NormalMap;
                Normal                       = mix(Normal, texture(PrevLayerNormalMap, PrevLayerTextureCoords).rgb * 2 - 1, BlendFactor);
            }
        }
    }
    
    LightContribution LightCntrb;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        ShadowFactor = CalculateShadow(fsIn.FragPos, Normal, normalize(uLightDirection));
        LightCntrb   = CalculateDirectionalLightContribution(ToViewN, Normal, 32);
    }
    else if (uLightType == POINT_LIGHT)
    {
        ShadowFactor = CalculateShadowCubemap(fsIn.FragPos, Normal, uLightPosition);
        LightCntrb   = CalculatePointLightContribution(fsIn.FragPos, ToViewN, Normal, 32);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        ShadowFactor = CalculateShadow(fsIn.FragPos, Normal, normalize(uLightDirection));
        LightCntrb   = CalculateSpotLightContribution(fsIn.FragPos, ToViewN, Normal, 32);
    }

    
    vec3 FragColor =  TerrainDiffuse * (LightCntrb.Diffuse + LightCntrb.Specular) * LightCntrb.Attenuation;
    oFragColor     = vec4((FragColor * ShadowFactor) + FragColor * uAmbientStrength, 1);
}