#version 450 core

#include "common.glsl"
#include "blinn_phong_uniforms.glsl"
#include "batch_instance.glsl"

in VS_OUT
{
    vec3 InterpolatedNormal;
    vec3 FragPos;
}
fsIn;

#include "lights.glsl"


out vec4 oFragColor;

#include "shadow_mapping.glsl"

void main()
{
    vec3 normal = normalize(fsIn.InterpolatedNormal);
    vec3 toViewN = normalize(uViewPos - fsIn.FragPos);

    vec3 ambient = MATERIAL_DATA.DiffuseColor * uAmbientStrength;
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, MATERIAL_DATA.Shininess);
    }
    else if (uLightType == POINT_LIGHT)
    {
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLightPosition);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, MATERIAL_DATA.Shininess);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, MATERIAL_DATA.Shininess);
    }

    vec3 fragColor = (MATERIAL_DATA.DiffuseColor *  lightCntrb.Diffuse) + (MATERIAL_DATA.SpecularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 1);
}