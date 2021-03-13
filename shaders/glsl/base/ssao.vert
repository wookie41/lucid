#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 3) in vec2 aTextureCoords;

out vec2 inTextureCoords;

void main()
{
    inTextureCoords = aTextureCoords;
    gl_Position = vec4(aPosition, 1);
}