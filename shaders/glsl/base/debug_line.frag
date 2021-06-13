#version 450 core

in vec3 inColor;
out vec4 oFragColor;

void main()
{
    oFragColor = vec4(inColor, 1);
}