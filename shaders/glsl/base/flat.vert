#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"

in int gl_InstanceID;
flat out int InstanceID;

layout(location = 0) in vec3 aPosition;

void main()
{
    InstanceID = gl_InstanceID;
    vec4 WorldPos = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1.0);
    gl_Position = uProjection * uView * WorldPos;
}
