#version 330 core

in VS_OUT
{
    vec3 Normal;
    vec3 FragPos;
}
fsIn;

#include "lights.glsl"
#include "material.glsl"
#include "shadow_mapping.glsl"

uniform float uAmbientStrength;
uniform vec3 uViewPos;

uniform BlinnPhongMaterial uMaterial;

out vec4 oFragColor;

void main()
{
    vec3 normal = normalize(fsIn.Normal);
    vec3 toViewN = normalize(uViewPos - fsIn.FragPos);

    vec3 ambient = uMaterial.DiffuseColor * uAmbientStrength;
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLight.Type == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == POINT_LIGHT)
    {

        shadowFactor = CalculateShadow(fsIn.FragPos, normal, uLight.Direction);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }

    vec3 fragColor = (uMaterial.DiffuseColor *  lightCntrb.Diffuse) + (uMaterial.SpecularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 0);
}