#version 450 core

#include "common_ssbo.glsl"

in int gl_InstanceID;
flat out int InstanceID;

layout(location = 0) in vec3 aPosition;


void main() 
{
    gl_Position = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1.0);
}