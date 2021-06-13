#version 450 core

#include "forward_prepass_common.glsl"

in int gl_InstanceID;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTextureCoords;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 PositionVS;
out vec3 NormalVS;
out vec2 TexCoords;
flat out int InstanceId;
flat out mat3 TBNMatrix;


void main() 
{
    InstanceId = gl_InstanceID;
    TexCoords = aTextureCoords;
    
    mat3 NormalMatrix = transpose(inverse(mat3(uView * MODEL_MATRIX)));

    vec4 WorldPos = MODEL_MATRIX * vec4(aPosition, 1.0);
    PositionVS = (uView * WorldPos).xyz;

    vec3 N = normalize(NormalMatrix * (PREPASS_DATA.NormalMultiplier * aNormal));
    vec3 T = normalize(NormalMatrix * (PREPASS_DATA.NormalMultiplier * aTangent));

    T = normalize(T - (dot(T, N) * N)); // make them orthonormal
    vec3 B = normalize(cross(N, T));
    TBNMatrix = mat3(T, B, N);
    NormalVS = N;
    
    gl_Position = uProjection * uView * WorldPos;
}