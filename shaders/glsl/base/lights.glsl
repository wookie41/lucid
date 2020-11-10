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
    float Constant;
    float Linear;
    float Quadratic;
    vec3 Color;
    float InnerCutOffCos;
    float OuterCutOffCos;
};

vec3 CalculateDirectionalLightContribution(in vec3 ToView,
                                           in vec3 Normal,
                                           in vec3 FragDiffuseColor,
                                           in vec3 FragSpecularColor,
                                           in vec3 LightDir,
                                           in vec3 LightColor,
                                           in int Shininess)
{
    vec3 toLightDirN = normalize(-LightDir);
    vec3 toViewN = normalize(ToView);

    float diffuseStrength = max(dot(Normal, toLightDirN), 0.0);
    vec3 diffuse = diffuseStrength * FragDiffuseColor * LightColor;

    vec3 halfWay = normalize(ToView + (-LightDir));
    float specularStrength = pow(max(dot(Normal, halfWay), 0.0), Shininess);
    vec3 specular = specularStrength * FragSpecularColor * LightColor;

    return diffuse + specular;
}

vec3 CalculatePointLightContribution(in vec3 FragPos,
                                     in vec3 ToView,
                                     in vec3 Normal,
                                     in vec3 LightPos,
                                     in vec3 LightColor,
                                     in vec3 LightDir,
                                     in float Constant,
                                     in float Linear,
                                     in float Quadratic,
                                     in vec3 FragDiffuseColor,
                                     in vec3 FragSpecularColor,
                                     in int Shininess)
{
    float distanceToLight = length(FragPos - LightPos);
    vec3 color =
      CalculateDirectionalLightContribution(ToView, Normal, FragDiffuseColor, FragSpecularColor, LightDir, LightColor, Shininess);
    return color * (1.0 / (Constant + (Linear * distanceToLight) + (Quadratic * (distanceToLight * distanceToLight))));
}

vec3 CalculateSpotLightContribution(in vec3 FragPos,
                                    in SpotLight InSpotLight,
                                    in vec3 ToView,
                                    in vec3 Normal,
                                    in vec3 FragDiffuseColor,
                                    in vec3 FragSpecularColor,
                                    in int Shininess)
{
    vec3 toLight = normalize(InSpotLight.Position - FragPos);
    vec3 lightDir = normalize(InSpotLight.Direction);

    float epsilon = InSpotLight.InnerCutOffCos - InSpotLight.OuterCutOffCos;
    float theta = dot(toLight, -InSpotLight.Direction);

    float intensity = clamp((theta - InSpotLight.OuterCutOffCos) / epsilon, 0.0, 1.0);

    return intensity * CalculatePointLightContribution(FragPos, ToView, Normal, InSpotLight.Position, InSpotLight.Color, lightDir,
                                                       InSpotLight.Constant, InSpotLight.Linear, InSpotLight.Quadratic,
                                                       FragDiffuseColor, FragSpecularColor, Shininess);
}