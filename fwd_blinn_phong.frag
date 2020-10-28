#version 330 core

in VS_OUT
{
    vec3 Normal;
    vec3 WorldPos;
}
fsIn;

struct DirectionalLight
{
    vec3 Direction;
    vec3 Color;
};

uniform DirectionalLight uLight;
uniform float uAmbientStrength;
uniform vec3 uViewPos;

struct Material
{
    int Shininess;
    vec3 DiffuseColor;
    vec3 SpecularColor;
};

uniform Material uMaterial;

vec3 CalculateDirectionalLightContribution();

out vec4 oFragColor;

void main()
{
    vec3 ambient = uMaterial.DiffuseColor * uAmbientStrength;
    vec3 fragColor = ambient + CalculateDirectionalLightContribution();
    oFragColor = vec4(fragColor, 1.0);
}

vec3 CalculateDirectionalLightContribution()
{
    vec3 normal = normalize(fsIn.Normal); // normalize the interpolated normal
    vec3 toView = normalize(uViewPos - fsIn.WorldPos);
    vec3 lightDir = normalize(-uLight.Direction);

    float diffuseStrength = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = uMaterial.DiffuseColor * diffuseStrength * uLight.Color;

    vec3 reflectedToLight = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(toView, reflectedToLight), 0.0), uMaterial.Shininess);
    vec3 specular = specularStrength * uMaterial.SpecularColor * uLight.Color;

    return diffuse + specular;
}