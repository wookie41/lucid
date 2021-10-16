#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"
#include "pbr_lights.glsl"

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

    bool bHasAOMap;
    bool bHasNormalMap;
    bool bHasDisplacementMap;
};

#include "pbr.glsl"

layout(std430, binding = 3) buffer MaterialDataDataBlock { FTexturedPBRMaterial MaterialData[]; };

#include "parallax_occlusion.glsl"

out vec4 oFragColor;

void main()
{
    vec3 ToViewN = normalize(uViewPos - fsIn.FragPos);
    vec2 UV      = fsIn.TextureCoords;

    if (MATERIAL_DATA.bHasDisplacementMap)
    {
        UV = ParallaxOcclusionMapping(fsIn.inverseTBN * ToViewN, fsIn.TextureCoords, MATERIAL_DATA.DisplacementMap);
        if (UV.x > 1 || UV.x < 0 || UV.y > 1 || UV.y < 0)
        {
            discard;
        }
    }

    vec3 Normal;
    if (MATERIAL_DATA.bHasNormalMap)
    {
        Normal = normalize(fsIn.TBN * ((texture(MATERIAL_DATA.NormalMap, UV).rgb * 2) - 1));
    }
    else
    {
        Normal = normalize(fsIn.InterpolatedNormal);
    }

    float Roughness = texture(MATERIAL_DATA.RoughnessMap, UV).r;
    float Metallic  = texture(MATERIAL_DATA.MetallicMap, UV).r;
    vec3  Albedo    = texture(MATERIAL_DATA.AlbedoMap, UV).rgb;

    vec2 ScreenSpaceCoords = (gl_FragCoord.xy / uViewportSize);

    float AmbientOcclusion = MATERIAL_DATA.bHasAOMap ? texture(MATERIAL_DATA.AOMap, UV).r : texture(uAmbientOcclusion, ScreenSpaceCoords).r;
    vec3  Ambient          = Albedo * uAmbientStrength * AmbientOcclusion;

    oFragColor = vec4(Ambient + CalculatePBR(Normal, Roughness, Metallic, Albedo), 1.0);
}