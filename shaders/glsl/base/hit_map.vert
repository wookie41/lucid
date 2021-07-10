#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"

layout (location = 0) in vec3 aPosition;

out int InstanceID;
out vec4 oWorldPos;

void main()
{
    InstanceID = gl_InstanceID;
    vec4 WorldPos = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1);
    oWorldPos = WorldPos;
    gl_Position = uProjection * uView * WorldPos;
}