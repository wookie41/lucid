#version 450 core

layout(location = 0) in vec3 aPosition;

#include "batch_instance.glsl"

in int gl_InstanceID;
flat out int InstanceID;

uniform mat4 uLightMatrix;

void main()
{
    InstanceID = gl_InstanceID;
    gl_Position = uLightMatrix * INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1.0);
}
