#version 420 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform bool uReverseNormals;

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
    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    vec4 worldPos = uModel * vec4(aPosition, 1);

    vec3 N = normalize(normalMatrix * (uReverseNormals ? -1.0 * aNormal : aNormal));
    vec3 T = normalize(normalMatrix * (uReverseNormals ? -1.0 * aTangent : aTangent));
    T = normalize(T - (dot(T, N) * N)); // make them orthonormal
    vec3 B = normalize(cross(N, T));

    vsOut.FragPos = worldPos.xyz;
    vsOut.TextureCoords = aTextureCoords;
    vsOut.TBN = mat3(T, B, N);
    vsOut.inverseTBN = transpose(vsOut.TBN);
    vsOut.InterpolatedNormal = N;

    gl_Position = uProjection * uView * worldPos;
}
