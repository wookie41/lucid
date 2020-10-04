#version 330 core

// Shader constants //

#define MAX_DIRECTIONAL_LIGHTS 32

///////////////////////////////

// Shader inputs //

in vec3 iNormal;
in vec3 iWorldPos;

///////////////////////////////

// Shader outputs //

out vec4 oFragColor;

// Lights types //

struct DirectionalLight
{
    vec3 Direction;
    vec3 Color;
};

///////////////////////////////


// Shader-wide properties

uniform float uAmbientStrength;
uniform vec3 uViewPos;

uniform int uNumOfDirectionalLight;
uniform DirectionalLight uDirectionalLights[MAX_DIRECTIONAL_LIGHTS];

///////////////////////////////


// Material properties

uniform vec3 uDiffuseColor;
uniform vec3 uSpecularColor;
uniform float uShininess;

///////////////////////////////


// Lighting function //

vec3 CalculateDirectionalLightContribution(in vec3 ToView, in vec3 Normal, in DirectionalLight Light);

///////////////////////////////

void main()
{
    // normalize the interpolated normal
    vec3 normal = normalize(iNormal);
    vec3 toView = normalize(uViewPos - iWorldPos);

    vec3 ambient = uDiffuseColor * uAmbientStrength;

    int numOfDirectionalLight = uNumOfDirectionalLight > MAX_DIRECTIONAL_LIGHTS ? 
        MAX_DIRECTIONAL_LIGHTS : 
        uNumOfDirectionalLight;

    vec3 fragColor = ambient;

    for (int lightIdx = 0; lightIdx < numOfDirectionalLight; ++lightIdx)
    {
        fragColor += CalculateDirectionalLightContribution(toView, normal, uDirectionalLights[lightIdx]);
    }

    oFragColor = vec4(fragColor, 1.0);
}

vec3 CalculateDirectionalLightContribution(in vec3 ToView, in vec3 Normal, in DirectionalLight Light)
{
    vec3 lightDir = normalize(-Light.Direction);
    
    float diffuseStrength = max(dot(lightDir, Normal), 0.0);
    vec3 diffuse = uDiffuseColor * diffuseStrength * Light.Color;

    vec3 reflectedToLight = reflect(-lightDir, Normal);
    float specularStrength = pow(max(dot(ToView, reflectedToLight), 0.0), uShininess);
    vec3 specular = specularStrength * uSpecularColor* Light.Color;

    return diffuse + specular;
}