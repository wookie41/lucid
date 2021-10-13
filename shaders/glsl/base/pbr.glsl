const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 FresnelSchlick(float cosTheta, vec3 F0) { return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }

vec3 CalculatePBR(vec3 InNormal, float InRoughness, float InMetallic, vec3 InAlbedo)
{
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, InAlbedo, InMetallic);

    vec3 L = vec3(1);
    vec3 V = normalize(uViewPos - fsIn.FragPos);

    // calculate light radiance
    vec3 Radiance = vec3(1);
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        L        = -uLightDirection;
        Radiance = CalculateDirectionalLightRadiance(V, InNormal);
    }
    else if (uLightType == POINT_LIGHT)
    {
        L        = normalize(uLightPosition - fsIn.FragPos);
        Radiance = CalculatePointLightRadiance(fsIn.FragPos, V, InNormal);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        L        = normalize(uLightPosition - fsIn.FragPos);
        Radiance = CalculateSpotLightRadiance(fsIn.FragPos, V, InNormal);
    }

    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(InNormal, H, InRoughness);
    float G   = GeometrySmith(InNormal, V, L, InRoughness);
    vec3  F   = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3  numerator   = NDF * G * F;
    float denominator = 4 * max(dot(InNormal, V), 0.0) * max(dot(InNormal, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3  specular    = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - InMetallic;

    float NdotL = max(dot(InNormal, L), 0.0);

    vec3 Lo = (kD * InAlbedo / PI + specular) * Radiance * NdotL;

    float ao      = 0.1;
    vec3  ambient = vec3(0.03) * InAlbedo * ao;

    return ambient + Lo;
}