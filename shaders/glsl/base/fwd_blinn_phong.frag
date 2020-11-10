#version 330 core

#include "lights.glsl"
#include "material.glsl"

in VS_OUT
{
    vec3 Normal;
    vec3 WorldPos;
}
fsIn;

uniform int uLightToUse;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;
uniform SpotLight uSpotLight;

uniform float uAmbientStrength;
uniform vec3 uViewPos;

uniform BlinnPhongMaterial uMaterial;

out vec4 oFragColor;

void main()
{
    vec3 normal = normalize(fsIn.Normal); // normalize the interpolated normal
    vec3 toView = uViewPos - fsIn.WorldPos;

    vec3 ambient = uMaterial.DiffuseColor * uAmbientStrength;
    vec3 fragColor = ambient;

    if (uLightToUse == DIRECTIONAL_LIGHT)
    {

        fragColor += CalculateDirectionalLightContribution(toView, normal, uMaterial.DiffuseColor, uMaterial.SpecularColor,
                                                           uDirectionalLight.Direction, uDirectionalLight.Color, uMaterial.Shininess);
    }
    else if (uLightToUse == POINT_LIGHT)
    {

        fragColor += CalculatePointLightContribution(
          fsIn.WorldPos, toView, normal, uPointLight.Position, uPointLight.Color, normalize(fsIn.WorldPos - uPointLight.Position),
          uPointLight.Constant, uPointLight.Linear, uPointLight.Quadratic, uMaterial.DiffuseColor, uMaterial.SpecularColor, uMaterial.Shininess);
    }
    else if (uLightToUse == SPOT_LIGHT)
    {
        fragColor += CalculateSpotLightContribution(fsIn.WorldPos, uSpotLight, toView, normal, uMaterial.DiffuseColor,
                                                    uMaterial.SpecularColor, uMaterial.Shininess);
    }

    oFragColor = vec4(fragColor, 1.0);
}