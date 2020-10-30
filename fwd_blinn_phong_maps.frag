#version 330 core

#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

struct DirectionalLight
{
    vec3 Direction;
    vec3 Color;
};

struct PointLight
{
    vec3 Position;
    float Constant;
    float Linear;
    float Quadratic;
    vec3 Color;
};

struct Material
{
    int Shininess;
    sampler2D DiffuseMap;
    sampler2D SpecularMap;
    sampler2D NormalMap;
};

in VS_OUT
{
    vec3 TanFragPos;
    flat vec3 TanViewPos;
    flat vec3 TanDirectionalLightDir;
    flat vec3 TanPointLightPos;
    vec2 TextureCoords;
}
fsIn;

uniform float uAmbientStrength;

uniform Material uMaterial;

uniform int uLightToUse;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;

vec3 CalculateDirectionalLightContribution(vec3 TanToView,
                                           vec3 TanFragNormal,
                                           in vec3 FragDiffuseColor,
                                           in vec3 FragSpecularColor,
                                           vec3 TanLightDir,
                                           vec3 LightColor);
vec3 CalculatePointLigthContribution(vec3 TanToView, vec3 TanFragNormal, in vec3 FragDiffuseColor, in vec3 FragSpecularColor);

out vec4 oFragColor;

void main()
{
    vec3 tanToView = normalize(fsIn.TanViewPos - fsIn.TanFragPos);

    vec3 tanNormal = normalize((texture(uMaterial.NormalMap, fsIn.TextureCoords).rgb * 2) - 1); // normals are in tangent space
    vec3 diffuseColor = texture(uMaterial.DiffuseMap, fsIn.TextureCoords).rgb;
    vec3 specularColor = texture(uMaterial.SpecularMap, fsIn.TextureCoords).rgb;

    vec3 ambient = diffuseColor * uAmbientStrength;
    vec3 fragColor = ambient;

    if (uLightToUse == DIRECTIONAL_LIGHT)
    {
        fragColor += CalculateDirectionalLightContribution(tanToView, tanNormal, diffuseColor, specularColor,
                                                           fsIn.TanDirectionalLightDir, uDirectionalLight.Color);
    }
    else if (uLightToUse == POINT_LIGHT)
    {
        fragColor += CalculatePointLigthContribution(tanToView, tanNormal, diffuseColor, specularColor);
    }
    else if (uLightToUse == SPOT_LIGHT)
    {
        //@TODO
    }

    oFragColor = vec4(fragColor, 1.0);
}

vec3 CalculateDirectionalLightContribution(vec3 TanToView,
                                           vec3 TanNormal,
                                           in vec3 FragDiffuseColor,
                                           in vec3 FragSpecularColor,
                                           vec3 TanLightDir,
                                           vec3 LightColor)
{
    vec3 toLightDir = normalize(-TanLightDir);

    float diffuseStrength = max(dot(TanNormal, toLightDir), 0.0);
    vec3 diffuse = diffuseStrength * FragDiffuseColor * LightColor;

    vec3 reflectedLightDir = reflect(-toLightDir, TanNormal);
    float specularStrength = pow(max(dot(TanToView, reflectedLightDir), 0.0), uMaterial.Shininess);
    vec3 specular = specularStrength * FragSpecularColor * LightColor;

    return diffuse + specular;
}

vec3 CalculatePointLigthContribution(vec3 TanToView, vec3 TanNormal, in vec3 FragDiffuseColor, in vec3 FragSpecularColor)
{
    vec3 tanLightDir = fsIn.TanFragPos - fsIn.TanPointLightPos;
    float tanLightDistance = length(tanLightDir);
    vec3 color = CalculateDirectionalLightContribution(TanToView, TanNormal, FragDiffuseColor, FragDiffuseColor,
                                                       normalize(tanLightDir), uPointLight.Color);
    return color * (1.0 / (uPointLight.Constant + (uPointLight.Linear * tanLightDistance) +
                           (uPointLight.Quadratic * (tanLightDistance * tanLightDistance))));
}
