#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3
#define MAX_SHADOW_CASCADES 6

uniform int  uLightType;
uniform vec3 uLightColor;
uniform vec3 uLightPosition;

uniform vec3 uLightDirection;

uniform float uLightAttenuationRadius;
uniform float uLightInvAttenuationRadiusSquared;
uniform float uLightIntensity;

uniform float uLightInnerCutOffCos;
uniform float uLightOuterCutOffCos;

uniform mat4  uLightMatrix;
uniform float uLightFarPlane;

uniform bool        uLightCastsShadows;
uniform sampler2D   uLightShadowMap;
uniform samplerCube uLightShadowCube;

uniform int       uCascadeCount;
uniform mat4      uCascadeMatrices[MAX_SHADOW_CASCADES];
uniform float     uCascadeFarPlanes[MAX_SHADOW_CASCADES];
uniform sampler2D uCascadeShadowMaps[MAX_SHADOW_CASCADES];
uniform vec3      uCascadeFrustumPositions[MAX_SHADOW_CASCADES];

vec3 CalculateDirectionalLightRadiance(in vec3 ToViewN, in vec3 Normal)
{
    vec3  ToLight = -uLightDirection;
    float NdotL   = max(dot(Normal, ToLight), 0.0);
    return NdotL * uLightColor * uLightIntensity;
}

vec3 CalculatePointLightRadiance(in vec3 FragPos, in vec3 ToViewN, in vec3 Normal)
{
    vec3  LightDirN       = normalize(FragPos - uLightPosition);
    float distanceToLight = length(FragPos - uLightPosition);
    float NdotL           = max(dot(Normal, -LightDirN), 0.0);

    vec3 radiance = NdotL * uLightColor;

    float distanceSquare = distanceToLight * distanceToLight;
    float factor         = distanceSquare * uLightInvAttenuationRadiusSquared;
    float smoothFactor   = max(1.0 - factor * factor, 0.0);
    float Attenuation    = (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);

    return radiance * Attenuation * uLightIntensity;
}

vec3 CalculateSpotLightRadiance(in vec3 FragPos, in vec3 ToViewN, in vec3 Normal)
{
    vec3 toLightN = normalize(uLightPosition - FragPos);

    float epsilon   = uLightInnerCutOffCos - uLightOuterCutOffCos;
    float theta     = dot(toLightN, normalize(-uLightDirection));
    float intensity = clamp((theta - uLightOuterCutOffCos) / epsilon, 0.0, 1.0);
    
    vec3 radiance = CalculatePointLightRadiance(FragPos, ToViewN, Normal);
    return radiance * intensity;
}