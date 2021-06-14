#version 450 core

#include "common_ssbo.glsl"

in int gl_InstanceID;
flat out int InstanceID;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoords;

uniform mat4 uView;
uniform mat4 uProjection;

out VS_OUT
{
    vec3 InterpolatedNormal;
    vec3 FragPos;
}
vsOut;

void main()
{
    InstanceID = gl_InstanceID;

    mat3 normalMatrix = mat3(transpose(inverse(INSTANCE_DATA.ModelMatrix)));
    vec4 FragPos = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1);

    vsOut.FragPos = FragPos.xyz;
    vsOut.InterpolatedNormal = normalMatrix * (aNormal * INSTANCE_DATA.NormalMultiplier);

    gl_Position = uProjection * uView * FragPos;
}
