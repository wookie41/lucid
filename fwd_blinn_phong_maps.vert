#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoords;

struct DirectionalLight
{
    vec3 Direction;
    vec3 Color;
};

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uViewPos;
uniform DirectionalLight uLight;

out VS_OUT
{
    vec3 TangentSpaceFragPos;
    vec3 TangentSpaceViewPos;
    DirectionalLight TangentSpaceLight;
    vec2 TextureCoords;
} vsOut;

void main()
{
    mat3 modelMatrix = mat3(transpose(inverse(uModel)));
    
    vec3 T = normalize(aTangent * modelMatrix);
    vec3 N = normalize(aNormal * modelMatrix);
    T = normalize(T - (dot(T, N) * N)); // make them orthonormal
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    vec4 worldPos = uModel * vec4(aPosition, 1);

    vsOut.TangentSpaceFragPos = TBN * worldPos.xyz;
    vsOut.TangentSpaceViewPos = TBN * uViewPos;
    vsOut.TextureCoords = aTextureCoords;

    vsOut.TangentSpaceLight.Direction = TBN * uLight.Direction;
    vsOut.TangentSpaceLight.Color = uLight.Color;

    gl_Position = uProjection * uView * worldPos;
}
