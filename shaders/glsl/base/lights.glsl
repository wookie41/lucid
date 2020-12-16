#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

struct Light
{
    int Type;
    vec3 Direction;
    vec3 Color;
    vec3 Position;
    float Constant;
    float Linear;
    float Quadratic;
    float InnerCutOffCos;
    float OuterCutOffCos;
    mat4 LightSpaceMatrix;
    bool CastsShadows;
    sampler2D ShadowMap;
    float FarPlane;
};

uniform samplerCube uShadowCube;

uniform Light uLight;

struct LightContribution
{
    float Attenuation;
    vec3 Diffuse;
    vec3 Specular;
};

LightContribution _CalculateDirectionalLightContribution(in vec3 ToViewN,
                                           in vec3 Normal,
                                           in vec3 LightDirN,
                                           in int Shininess)
{
    float diffuseStrength = max(dot(Normal, -LightDirN), 0.0);
    vec3 diffuse = diffuseStrength * uLight.Color;

    vec3 halfWayN = normalize((-LightDirN) + ToViewN);
    float specularStrength = pow(max(dot(Normal, halfWayN), 0.0), Shininess);
    vec3 specular = specularStrength * uLight.Color;

    return LightContribution(1, diffuse, specular);
}

LightContribution CalculateDirectionalLightContribution(in vec3 ToViewN,
                                           in vec3 Normal,
                                           in int Shininess)
{
    return _CalculateDirectionalLightContribution(ToViewN, Normal, normalize(uLight.Direction), Shininess);
}

LightContribution _CalculatePointLightContribution(in vec3 FragPos,
                                     in vec3 ToViewN,
                                     in vec3 Normal,
                                     in vec3 LightDirN,
                                     in int Shininess)
{
    float distanceToLight = length(FragPos - uLight.Position);
    LightContribution ctrb = _CalculateDirectionalLightContribution(ToViewN, Normal, LightDirN, Shininess);
    float attenuation = 1.0 / (uLight.Constant + (uLight.Linear * distanceToLight) + (uLight.Quadratic * (distanceToLight * distanceToLight)));
    return LightContribution(attenuation, ctrb.Diffuse * attenuation, ctrb.Specular * attenuation);
}


LightContribution CalculatePointLightContribution(in vec3 FragPos,
                                     in vec3 ToViewN,
                                     in vec3 Normal,
                                     in int Shininess)
{
    return _CalculatePointLightContribution(FragPos, ToViewN, Normal, normalize(FragPos - uLight.Position), Shininess);
}


LightContribution CalculateSpotLightContribution(in vec3 FragPos,
                                    in vec3 ToViewN,
                                    in vec3 Normal,
                                    in int Shininess)
{
    vec3 toLightN = normalize(uLight.Position - FragPos);

    float epsilon = uLight.InnerCutOffCos - uLight.OuterCutOffCos;
    float theta = dot(toLightN, normalize(-uLight.Direction));
    float intensity = clamp((theta - uLight.OuterCutOffCos) / epsilon, 0.0, 1.0);

    LightContribution ctrb = _CalculatePointLightContribution(FragPos, ToViewN, Normal, normalize(uLight.Direction), Shininess);
    return LightContribution(ctrb.Attenuation, ctrb.Diffuse * intensity, ctrb.Specular * intensity);
}