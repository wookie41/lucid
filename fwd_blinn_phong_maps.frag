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

struct SpotLight
{
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float Constant;
    float Linear;
    float Quadratic;
    float InnerCutOffCos;
    float OuterCutOffCos;
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
    vec3 FragPos;
    vec2 TextureCoords;
    mat3 TBN;
}
fsIn;

uniform float uAmbientStrength;

uniform Material uMaterial;

uniform int uLightToUse;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;
uniform SpotLight uSpotLight;

uniform vec3 uViewPos;

vec3 CalculateDirectionalLightContribution(in vec3 ToView,
                                           in vec3 FragNormal,
                                           in vec3 FragDiffuseColor,
                                           in vec3 FragSpecularColor,
                                           in vec3 LightDir,
                                           in vec3 LightColor);

vec3 CalculatePointLightContribution(in vec3 ToView,
                                     in vec3 Normal,
                                     in vec3 LightPos,
                                     in vec3 LightColor,
                                     in vec3 LightDir,
                                     in float Constant,
                                     in float Linear,
                                     in float Quadratic,
                                     in vec3 FragDiffuseColor,
                                     in vec3 FragSpecularColor);

vec3 CalculateSpotLightContribution(in vec3 ToView, in vec3 Normal, in vec3 FragDiffuseColor, in vec3 FragSpecularColor);

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
                                                           uDirectionalLight.Direction, uDirectionalLight.Color);
    }
    else if (uLightToUse == POINT_LIGHT)
    {
        fragColor += CalculatePointLightContribution(toView, normal, uPointLight.Position, uPointLight.Color,
                                                     normalize(fsIn.FragPos - uPointLight.Position), uPointLight.Constant,
                                                     uPointLight.Linear, uPointLight.Quadratic, diffuseColor, specularColor);
    }
    else if (uLightToUse == SPOT_LIGHT)
    {
        fragColor += CalculateSpotLightContribution(toView, normal, diffuseColor, specularColor);
    }

    oFragColor = vec4(fragColor, 1.0);
}

vec3 CalculateDirectionalLightContribution(in vec3 ToView,
                                           in vec3 Normal,
                                           in vec3 FragDiffuseColor,
                                           in vec3 FragSpecularColor,
                                           in vec3 LightDir,
                                           in vec3 LightColor)
{
    vec3 toLightDirN = normalize(-LightDir);
    vec3 toViewN = normalize(ToView);

    float diffuseStrength = max(dot(Normal, toLightDirN), 0.0);
    vec3 diffuse = diffuseStrength * FragDiffuseColor * LightColor;

    vec3 halfWay = normalize(ToView + (-LightDir));
    float specularStrength = pow(max(dot(Normal, halfWay), 0.0), uMaterial.Shininess);
    vec3 specular = specularStrength * FragSpecularColor * LightColor;

    return diffuse + specular;
}

vec3 CalculatePointLightContribution(in vec3 ToView,
                                     in vec3 Normal,
                                     in vec3 LightPos,
                                     in vec3 LightColor,
                                     in vec3 LightDir,
                                     in float Constant,
                                     in float Linear,
                                     in float Quadratic,
                                     in vec3 FragDiffuseColor,
                                     in vec3 FragSpecularColor)
{
    float distanceToLight = length(fsIn.FragPos - LightPos);
    vec3 color = CalculateDirectionalLightContribution(ToView, Normal, FragDiffuseColor, FragSpecularColor, LightDir, LightColor);
    return color * (1.0 / (Constant + (Linear * distanceToLight) + (Quadratic * (distanceToLight * distanceToLight))));
}

vec3 CalculateSpotLightContribution(in vec3 ToView, in vec3 Normal, in vec3 FragDiffuseColor, in vec3 FragSpecularColor)
{
    vec3 toLight = normalize(uSpotLight.Position - fsIn.FragPos);
    vec3 lightDir = normalize(uSpotLight.Direction);

    float epsilon = uSpotLight.InnerCutOffCos - uSpotLight.OuterCutOffCos;
    float theta = dot(toLight, -uSpotLight.Direction);

    float intensity = clamp((theta - uSpotLight.OuterCutOffCos) / epsilon, 0.0, 1.0);

    return intensity * CalculatePointLightContribution(ToView, Normal, uSpotLight.Position, uSpotLight.Color, lightDir,
                                                       uSpotLight.Constant, uSpotLight.Linear, uSpotLight.Quadratic,
                                                       FragDiffuseColor, FragSpecularColor);
}