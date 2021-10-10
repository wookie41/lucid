#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"

in int gl_InstanceID;
flat out int InstanceID;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoords;

out VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
    mat3 inverseTBN;
}
vsOut;

void main()
{
    InstanceID = gl_InstanceID;

    mat3 normalMatrix = transpose(inverse(mat3(INSTANCE_DATA.ModelMatrix)));
    vec4 worldPos = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1);

    vec3 N = normalize(normalMatrix * (aNormal * INSTANCE_DATA.NormalMultiplier));
    vec3 T = normalize(normalMatrix * (aTangent * INSTANCE_DATA.NormalMultiplier));
    T = normalize(T - (dot(T, N) * N)); // make them orthonormal
    vec3 B = normalize(cross(N, T));

    vsOut.FragPos = worldPos.xyz;
    vsOut.TextureCoords = aTextureCoords;
    vsOut.TBN = mat3(T, B, N);
    vsOut.inverseTBN = transpose(vsOut.TBN);
    vsOut.InterpolatedNormal = N;

    gl_Position = uProjection * uView * worldPos;
}
