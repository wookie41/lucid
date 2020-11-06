#version 330 core

#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

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
    float Constant;
    float Linear;
    float Quadratic;
    vec3 Color;
    float InnerCutOffCos;
    float OuterCutOffCos;
};

uniform int uLightToUse;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;
uniform SpotLight uSpotLight;

uniform float uAmbientStrength;
uniform vec3 uViewPos;

struct Material
{
    int Shininess;
    vec3 DiffuseColor;
    vec3 SpecularColor;
};

uniform Material uMaterial;

vec3 CalculateDirectionalLightContribution(in vec3 Normal, in vec3 ToView, in vec3 LightDir, in vec3 LightColor);
vec3 CalculatePointLightContribution(in vec3 Normal,
                                     in vec3 ToView,
                                     in vec3 LightPos,
                                     in vec3 LightColor,
                                     in vec3 LightDir,
                                     in float Constant,
                                     in float Linear,
                                     in float Quadratic);

vec3 CalculateSpotLightContribution(vec3 Normal, vec3 ToView);

out vec4 oFragColor;

void main()
{
    vec3 normal = normalize(fsIn.Normal); // normalize the interpolated normal
    vec3 toView = normalize(uViewPos - fsIn.WorldPos);

    vec3 ambient = uMaterial.DiffuseColor * uAmbientStrength;
    vec3 fragColor = ambient;

    if (uLightToUse == DIRECTIONAL_LIGHT)
    {
        fragColor += CalculateDirectionalLightContribution(normal, toView, uDirectionalLight.Direction, uDirectionalLight.Color);
    }
    else if (uLightToUse == POINT_LIGHT)
    {
        fragColor += CalculatePointLightContribution(normal, toView, uPointLight.Position, uPointLight.Color,
                                                     normalize(fsIn.WorldPos - uPointLight.Position), uPointLight.Constant,
                                                     uPointLight.Linear, uPointLight.Quadratic);
    }
    else if (uLightToUse == SPOT_LIGHT)
    {
        fragColor += CalculateSpotLightContribution(normal, toView);
    }

    oFragColor = vec4(fragColor, 1.0);
}

vec3 CalculateDirectionalLightContribution(in vec3 Normal, in vec3 ToView, in vec3 LightDir, in vec3 LightColor)
{
    vec3 lightDir = normalize(-LightDir);

    float diffuseStrength = max(dot(lightDir, Normal), 0.0);
    vec3 diffuse = uMaterial.DiffuseColor * diffuseStrength * LightColor;

    vec3 reflectedToLight = reflect(-lightDir, Normal);
    float specularStrength = pow(max(dot(ToView, reflectedToLight), 0.0), uMaterial.Shininess);
    vec3 specular = specularStrength * uMaterial.SpecularColor * LightColor;

    return diffuse + specular;
}

vec3 CalculatePointLightContribution(in vec3 Normal,
                                     in vec3 ToView,
                                     in vec3 LightPos,
                                     in vec3 LightColor,
                                     in vec3 LightDir,
                                     in float Constant,
                                     in float Linear,
                                     in float Quadratic)
{

    float distanceToLight = length(fsIn.WorldPos - LightPos);
    vec3 color = CalculateDirectionalLightContribution(Normal, ToView, LightDir, LightColor);
    return color * (1.0 / (Constant + (Linear * distanceToLight) + (Quadratic * (distanceToLight * distanceToLight))));
}

vec3 CalculateSpotLightContribution(in vec3 Normal, in vec3 ToView)
{
    vec3 toLight = normalize(uSpotLight.Position - fsIn.WorldPos);
    vec3 lightDir = normalize(uSpotLight.Direction);

    float epsilon = uSpotLight.InnerCutOffCos - uSpotLight.OuterCutOffCos;
    float theta = dot(toLight, -lightDir);
    
    float intensity = clamp((theta - uSpotLight.OuterCutOffCos) / epsilon, 0, 1);
    
    return intensity * CalculatePointLightContribution(Normal, ToView, uSpotLight.Position, uSpotLight.Color,
                                                       lightDir, uSpotLight.Constant, uSpotLight.Linear,
                                                       uSpotLight.Quadratic);
}
