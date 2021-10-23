#version 450 core
#include "common.glsl"
#include "batch_instance.glsl"
#include "pbr_lights.glsl"
#include "shadow_mapping.glsl"

#define HAS_ALBEDO 1
#define HAS_NORMAL 2
#define HAS_METALLIC 4
#define HAS_ROUGHNESS 8
#define HAS_AO 16
#define HAS_DISPLACEMENT 32

flat in int InstanceID;

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
    mat3 inverseTBN;
}
fsIn;

layout(bindless_sampler) uniform;

struct FTexturedPBRMaterial
{
    sampler2D RoughnessMap;
    sampler2D MetallicMap;
    sampler2D AlbedoMap;
    sampler2D AOMap;
    sampler2D NormalMap;
    sampler2D DisplacementMap;

    vec3  Albedo;
    float Roughness;
    float Metallic;

    int Flags;
};

#include "pbr.glsl"

layout(std430, binding = 3) buffer MaterialDataDataBlock { FTexturedPBRMaterial MaterialData[]; };

#include "parallax_occlusion.glsl"

out vec4 oFragColor;

void main()
{
    vec3 ToViewN = normalize(uViewPos - fsIn.FragPos);
    vec2 UV      = fsIn.TextureCoords;

    if (bool(MATERIAL_DATA.Flags & HAS_DISPLACEMENT))
    {
        UV = ParallaxOcclusionMapping(fsIn.inverseTBN * ToViewN, fsIn.TextureCoords, MATERIAL_DATA.DisplacementMap);
        if (UV.x > 1 || UV.x < 0 || UV.y > 1 || UV.y < 0)
        {
            discard;
        }
    }

    vec3 Normal;
    if (bool(MATERIAL_DATA.Flags & HAS_NORMAL))
    {
        Normal = normalize(fsIn.TBN * ((texture(MATERIAL_DATA.NormalMap, UV).rgb * 2) - 1));
    }
    else
    {
        Normal = normalize(fsIn.InterpolatedNormal);
    }

    float ShadowFactor = 1;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        ShadowFactor = CalculateShadow(fsIn.FragPos, Normal, normalize(uLightDirection));
    }
    else if (uLightType == POINT_LIGHT)
    {
        ShadowFactor = CalculateShadowCubemap(fsIn.FragPos, Normal, uLightPosition);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        ShadowFactor = CalculateShadow(fsIn.FragPos, Normal, normalize(uLightDirection));
    }

    vec3  Albedo    = bool(MATERIAL_DATA.Flags & HAS_ALBEDO) ? texture(MATERIAL_DATA.AlbedoMap, UV).rgb : MATERIAL_DATA.Albedo;
    float Roughness = bool(MATERIAL_DATA.Flags & HAS_ROUGHNESS) ? texture(MATERIAL_DATA.RoughnessMap, UV).r : MATERIAL_DATA.Roughness;
    float Metallic  = bool(MATERIAL_DATA.Flags & HAS_METALLIC) ? texture(MATERIAL_DATA.MetallicMap, UV).r : MATERIAL_DATA.Metallic;

    vec2 ScreenSpaceCoords = (gl_FragCoord.xy / uViewportSize);

    float AmbientOcclusion = bool(MATERIAL_DATA.Flags & HAS_AO) ? texture(MATERIAL_DATA.AOMap, UV).r : texture(uAmbientOcclusion, ScreenSpaceCoords).r;
    vec3  Ambient          = Albedo * uAmbientStrength * AmbientOcclusion;

    oFragColor = vec4(Ambient + CalculatePBR(Normal, Roughness, Metallic, Albedo) * ShadowFactor, 1.0);
}