#version 450 core

#include "common_ssbo.glsl"

in int gl_InstanceID;

layout(location = 0) in vec3 aPosition;


void main() 
{
    int InstanceID;
    InstanceID = gl_InstanceID;

    gl_Position = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1.0);
}