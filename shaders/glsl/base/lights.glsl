#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

uniform int     uLightType;
uniform vec3    uLightColor;
uniform vec3    uLightPosition;

uniform vec3    uLightDirection;

uniform float   uLightAttenuationRadius;

uniform float   uLightInnerCutOffCos;
uniform float   uLightOuterCutOffCos;

uniform mat4    uLightMatrix;
uniform float   uLightFarPlane;

uniform bool        uLightCastsShadows;
uniform sampler2D   uLightShadowMap;
uniform samplerCube uLightShadowCube;

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
    vec3 diffuse = diffuseStrength * uLightColor;

    vec3 specular = vec3(0);
    // Cut specular contribution on back-facing sides
    if(dot(fsIn.InterpolatedNormal, -LightDirN) > 0)
    {
        vec3 halfWayN = normalize(ToViewN + (-LightDirN));
        float specularStrength = pow(max(dot(Normal, halfWayN), 0.0), Shininess);
        specular = specularStrength * uLightColor;
    }

    return LightContribution(1, diffuse, specular);
}

LightContribution CalculateDirectionalLightContribution(in vec3 ToViewN,
                                           in vec3 Normal,
                                           in int Shininess)
{
    return _CalculateDirectionalLightContribution(ToViewN, Normal, normalize(uLightDirection), Shininess);
}

LightContribution _CalculatePointLightContribution(in vec3 FragPos,
                                     in vec3 ToViewN,
                                     in vec3 Normal,
                                     in vec3 LightDirN,
                                     in int Shininess)
{
    float distanceToLight = length(FragPos - uLightPosition);
    LightContribution ctrb = _CalculateDirectionalLightContribution(ToViewN, Normal, LightDirN, Shininess);
    float attenuation = 1.0 - (distanceToLight / uLightAttenuationRadius);
    return LightContribution(attenuation, ctrb.Diffuse * attenuation, ctrb.Specular * attenuation);
}


LightContribution CalculatePointLightContribution(in vec3 FragPos,
                                     in vec3 ToViewN,
                                     in vec3 Normal,
                                     in int Shininess)
{
    return _CalculatePointLightContribution(FragPos, ToViewN, Normal, normalize(FragPos - uLightPosition), Shininess);
}


LightContribution CalculateSpotLightContribution(in vec3 FragPos,
                                    in vec3 ToViewN,
                                    in vec3 Normal,
                                    in int Shininess)
{
    vec3 toLightN = normalize(uLightPosition - FragPos);

    float epsilon = uLightInnerCutOffCos - uLightOuterCutOffCos;
    float theta = dot(toLightN, normalize(-uLightDirection));
    float intensity = clamp((theta - uLightOuterCutOffCos) / epsilon, 0.0, 1.0);

    LightContribution ctrb = _CalculatePointLightContribution(FragPos, ToViewN, Normal, normalize(-toLightN), Shininess);
    return LightContribution(ctrb.Attenuation, ctrb.Diffuse * intensity, ctrb.Specular * intensity);
}