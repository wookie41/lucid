#version 330 core

struct DirectionalLight
{
    vec3 Direction;
    vec3 Color;
};

in VS_OUT
{
    vec3 TangentSpaceFragPos;
    vec3 TangentSpaceViewPos;
    DirectionalLight TangentSpaceLight;
    vec2 TextureCoords;
}
fsIn;

struct Material
{
    int Shininess;
    sampler2D DiffuseMap;
    sampler2D SpecularMap;
    sampler2D NormalMap;
};

uniform float uAmbientStrength;

uniform Material uMaterial;

vec3 CalculateDirectionalLightContribution(vec3 ToView, vec3 FragNormal, in vec3 FragDiffuseColor, in vec3 FragSpecularColor);

out vec4 oFragColor;

void main()
{
    vec3 toView = normalize(fsIn.TangentSpaceViewPos - fsIn.TangentSpaceFragPos);

    vec3 normal = normalize((texture(uMaterial.NormalMap, fsIn.TextureCoords).rgb * 2) - 1); // normals are in tangent space
    vec3 diffuseColor = texture(uMaterial.DiffuseMap, fsIn.TextureCoords).rgb;
    vec3 specularColor = texture(uMaterial.SpecularMap, fsIn.TextureCoords).rgb;

    vec3 ambient = diffuseColor * uAmbientStrength;
    vec3 fragColor = ambient + CalculateDirectionalLightContribution(toView, normal, diffuseColor, specularColor);

    oFragColor = vec4(fragColor, 1.0);
}

vec3 CalculateDirectionalLightContribution(vec3 ToView, vec3 FragNormal, in vec3 FragDiffuseColor, in vec3 FragSpecularColor)
{
    vec3 toLightDir = normalize(-fsIn.TangentSpaceLight.Direction);

    float diffuseStrength = max(dot(FragNormal, toLightDir), 0.0);
    vec3 diffuse = diffuseStrength * FragDiffuseColor * fsIn.TangentSpaceLight.Color;

    vec3 reflectedLightDir = reflect(-toLightDir, FragNormal);
    float specularStrength = pow(max(dot(ToView, reflectedLightDir), 0.0), uMaterial.Shininess);
    vec3 specular = specularStrength * FragSpecularColor * fsIn.TangentSpaceLight.Color;

    return diffuse + specular;
}