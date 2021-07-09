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

    vec3  Ambient      = vec3(1) * uAmbientStrength;
    float ShadowFactor = 1.0;

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

    vec3 FragColor = (vec3(1) * LightCntrb.Diffuse) + (vec3(1) * LightCntrb.Specular);
    oFragColor     = vec4((Ambient * LightCntrb.Attenuation) + (FragColor * ShadowFactor), 1);
}