#version 420 core

uniform vec4 uFlatColor;
out vec4 oFragColor;

void main()
{
    oFragColor = uFlatColor;
}