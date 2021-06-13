#version 450 core

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
    vec3 InterpolatedNormal;
    vec3 FragPos;
}
vsOut;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vec4 FragPos = uModel * vec4(aPosition, 1);

    vsOut.FragPos = FragPos.xyz;
    vsOut.InterpolatedNormal = normalMatrix * (uReverseNormals ? -1.0 * aNormal : aNormal);

    gl_Position = uProjection * uView * FragPos;
}
