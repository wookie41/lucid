#version 450 core

#include "common_ssbo.glsl"

in int gl_InstanceID;
flat out int InstanceID;

layout(location = 0) in vec3 aPosition;

uniform mat4 uLightMatrix;

void main() 
{
    InstanceID = gl_InstanceID;
    gl_Position = uLightMatrix * INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1.0);
}