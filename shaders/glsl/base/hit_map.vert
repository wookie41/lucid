#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"

layout (location = 0) in vec3 aPosition;

out int InstanceID;

void main()
{
    InstanceID = gl_InstanceID;
    gl_Position = uProjection * uView * INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1);
}