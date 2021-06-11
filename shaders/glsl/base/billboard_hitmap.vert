#version 420 core

layout(location = 0) in vec3 aPosition;

#include "billboard.glsl"

void main()
{
    gl_Position = CalculateBillboardPosition();
}
