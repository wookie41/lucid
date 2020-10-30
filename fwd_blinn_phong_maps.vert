#version 330 core

#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoords;

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

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uViewPos;

uniform int uLightToUse;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;
// uniform SpotLight uSpotLight;

out VS_OUT
{
    vec3 TanFragPos;
    flat vec3 TanViewPos;
    flat vec3 TanDirectionalLightDir;
    flat vec3 TanPointLightPos;
    vec2 TextureCoords;
}
vsOut;

void main()
{
    mat3 modelMatrix = mat3(transpose(inverse(uModel)));

    vec3 T = normalize(aTangent * modelMatrix);
    vec3 N = normalize(aNormal * modelMatrix);
    T = normalize(T - (dot(T, N) * N)); // make them orthonormal
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    vec4 worldPos = uModel * vec4(aPosition, 1);

    vsOut.TanFragPos = TBN * worldPos.xyz;
    vsOut.TanViewPos = TBN * uViewPos;
    vsOut.TextureCoords = aTextureCoords;

    if (uLightToUse == DIRECTIONAL_LIGHT)
    {
        vsOut.TanDirectionalLightDir = TBN * uDirectionalLight.Direction;
    }
    else if (uLightToUse == POINT_LIGHT)
    {
        vsOut.TanPointLightPos = TBN * uPointLight.Position;
    }
    else if (uLightToUse == SPOT_LIGHT)
    {
        //@TODO
    }

    gl_Position = uProjection * uView * worldPos;
}
