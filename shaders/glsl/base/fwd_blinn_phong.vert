#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangentSpace;
layout(location = 3) in vec2 aTextureCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out VS_OUT
{
    vec3 Normal;
    vec3 FragPos;
}
vsOut;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vec4 FragPos = uModel * vec4(aPosition, 1);

    vsOut.FragPos = FragPos.xyz;
    vsOut.Normal = normalMatrix * aNormal;

    gl_Position = uProjection * uView * FragPos;
}
