#version 450 core

in VS_OUT
{
    vec3 InterpolatedNormal;
    vec3 FragPos;
}
fsIn;

#include "lights.glsl"
#include "blinn_phong_uniforms.glsl"

uniform float uAmbientStrength;
uniform vec3 uViewPos;

out vec4 oFragColor;

#include "shadow_mapping.glsl"

void main()
{
    vec3 normal = normalize(fsIn.InterpolatedNormal);
    vec3 toViewN = normalize(uViewPos - fsIn.FragPos);

    vec3 ambient = uMaterialDiffuseColor * uAmbientStrength;
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterialShininess);
    }
    else if (uLightType == POINT_LIGHT)
    {
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLightPosition);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterialShininess);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterialShininess);
    }

    vec3 fragColor = (uMaterialDiffuseColor *  lightCntrb.Diffuse) + (uMaterialSpecularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 1);
}