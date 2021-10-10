#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"
#include "pbr_lights.glsl"
#include "pbr.glsl"

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

struct FPBRMaterial
{
    vec3  Albedo;
    float Metallic;
    float Roughness;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FPBRMaterial MaterialData[]; };

out vec4 oFragColor;

void main()
{
    vec3 N = normalize(fsIn.InterpolatedNormal);
    vec3 V = normalize(uViewPos - fsIn.FragPos);
    vec3 L = vec3(1);
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, MATERIAL_DATA.Albedo, MATERIAL_DATA.Metallic);
    
    // calculate light radiance
    vec3 radiance = vec3(1);
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        L = -uLightDirection;
        radiance = CalculateDirectionalLightRadiance(V, N);
    }
    else if (uLightType == POINT_LIGHT)
    {
        L = normalize(uLightPosition - fsIn.FragPos);
        radiance = CalculatePointLightRadiance(fsIn.FragPos, V, N);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        L = normalize(uLightPosition - fsIn.FragPos);
        radiance = CalculateSpotLightRadiance(fsIn.FragPos, V, N);
    }

    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, MATERIAL_DATA.Roughness);
    float G   = GeometrySmith(N, V, L, MATERIAL_DATA.Roughness);
    vec3  F   = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3  numerator   = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3  specular    = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - MATERIAL_DATA.Metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    vec3 Lo = (kD * MATERIAL_DATA.Albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    float ao  = 0.1;
    vec3 ambient = vec3(0.03) * MATERIAL_DATA.Albedo * ao;

    oFragColor = vec4(ambient + Lo, 1.0);
}