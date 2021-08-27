#version 450 core

#include "batch_instance.glsl"

in int gl_InstanceID;

layout(location = 0) in vec3 aPosition;

out vec4 FragPos;

uniform mat4 uLightSpaceMatrix;
void main()
{
    int InstanceID;
    InstanceID = gl_InstanceID;

    FragPos = INSTANCE_DATA.ModelMatrix * vec4(aPosition, 1.0);
    gl_Position = uLightSpaceMatrix * FragPos;
}