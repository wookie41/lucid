#version 420 core

layout(location = 0) in vec3 aPosition;
layout(location = 3) in vec2 aTextureCoords;

#include "billboard.glsl"

out vec2 oTextureCoords;

void main()
{
    oTextureCoords = aTextureCoords;
    gl_Position = CalculateBillboardPosition();
}
