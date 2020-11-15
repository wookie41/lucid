#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
}
vsOut;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - (dot(T, N) * N)); // make them orthonormal
    vec3 B = normalize(cross(N, T));

    vec4 worldPos = uModel * vec4(aPosition, 1);

    vsOut.FragPos = worldPos.xyz;
    vsOut.TextureCoords = aTextureCoords;
    vsOut.TBN = mat3(T, B, N);
    vsOut.InterpolatedNormal = normalMatrix * aNormal;

    gl_Position = uProjection * uView * worldPos;
}
