#version 330 core

#include "lights.glsl"
#include "material.glsl"

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    mat3 TBN;
}
fsIn;

uniform float uAmbientStrength;

uniform BlinnPhongMapsMaterial uMaterial;

uniform int uLightToUse;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;
uniform SpotLight uSpotLight;

uniform vec3 uViewPos;

out vec4 oFragColor;

void main()
{
    vec3 toView = uViewPos - fsIn.FragPos;

    vec3 normal = fsIn.TBN * normalize((texture(uMaterial.NormalMap, fsIn.TextureCoords).rgb * 2) - 1);
    vec3 diffuseColor = texture(uMaterial.DiffuseMap, fsIn.TextureCoords).rgb;
    vec3 specularColor = texture(uMaterial.SpecularMap, fsIn.TextureCoords).rgb;

    vec3 ambient = diffuseColor * uAmbientStrength;
    vec3 fragColor = ambient;

    if (uLightToUse == DIRECTIONAL_LIGHT)
    {
        fragColor += CalculateDirectionalLightContribution(toView, normal, diffuseColor, specularColor,
                                                           uDirectionalLight.Direction, uDirectionalLight.Color, uMaterial.Shininess);
    }
    else if (uLightToUse == POINT_LIGHT)
    {
        fragColor += CalculatePointLightContribution(fsIn.FragPos, toView, normal, uPointLight.Position, uPointLight.Color,
                                                     normalize(fsIn.FragPos - uPointLight.Position), uPointLight.Constant,
                                                     uPointLight.Linear, uPointLight.Quadratic, diffuseColor, specularColor, uMaterial.Shininess);
    }
    else if (uLightToUse == SPOT_LIGHT)
    {
        fragColor += CalculateSpotLightContribution(fsIn.FragPos, uSpotLight, toView, normal, diffuseColor, specularColor, uMaterial.Shininess);
    }

    oFragColor = vec4(fragColor, 1.0);
}