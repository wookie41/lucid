#version 450 core

#include "common.glsl"

layout(location = 0) in vec3 aPosition;

#include "billboard.glsl"

void main()
{
    gl_Position = CalculateBillboardPosition();
}
