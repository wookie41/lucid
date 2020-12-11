#version 330 core

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
}
fsIn;

#include "material.glsl"
#include "lights.glsl"
#include "shadow_mapping.glsl"

uniform float uAmbientStrength;

uniform BlinnPhongMapsMaterial uMaterial;

uniform vec3 uViewPos;

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
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(fsIn.FragPos - uLight.Position));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == POINT_LIGHT)
    {   
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(fsIn.FragPos - uLight.Position));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }

    vec3 fragColor = (diffuseColor * lightCntrb.Diffuse) + (specularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 0);
}