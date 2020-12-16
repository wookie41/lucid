#version 330 core

#include "material.glsl"

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
}
fsIn;

uniform float uAmbientStrength;
uniform vec3 uViewPos;
uniform BlinnPhongMapsMaterial uMaterial;

#include "lights.glsl"
#include "shadow_mapping.glsl"

out vec4 oFragColor;

void main()
{
    vec3 toViewN = normalize(uViewPos - fsIn.FragPos);

    vec3 normal;
    if (uMaterial.HasNormalMap)
    {
        normal = normalize(fsIn.TBN * ((texture(uMaterial.NormalMap, fsIn.TextureCoords).rgb * 2) - 1));
    }
    else
    {
        normal = normalize(fsIn.InterpolatedNormal);
    }

    vec3 diffuseColor =  texture(uMaterial.DiffuseMap, fsIn.TextureCoords).rgb;
    vec3 specularColor = uMaterial.HasSpecularMap ? texture(uMaterial.SpecularMap, fsIn.TextureCoords).rgb : uMaterial.SpecularColor; 

    vec3 ambient = diffuseColor * uAmbientStrength;
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLight.Type == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == POINT_LIGHT)
    {   
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLight.Position);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }

    vec3 fragColor = (diffuseColor * lightCntrb.Diffuse) + (specularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 0);
}