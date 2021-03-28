#version 330 core

in VS_OUT
{
    vec3 Normal;
    vec3 FragPos;
}
fsIn;

#include "lights.glsl"
#include "material.glsl"

uniform float uAmbientStrength;
uniform vec3 uViewPos;

uniform BlinnPhongMaterial uMaterial;

out vec4 oFragColor;

#include "shadow_mapping.glsl"

void main()
{
    vec3 normal = normalize(fsIn.Normal);
    vec3 toViewN = normalize(uViewPos - fsIn.FragPos);

    vec3 ambient = uMaterial.DiffuseColor * uAmbientStrength;
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterial.Shininess);
    }
    else if (uLightType == POINT_LIGHT)
    {
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLightPosition);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }

    vec3 fragColor = (uMaterial.DiffuseColor *  lightCntrb.Diffuse) + (uMaterial.SpecularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 0);
}