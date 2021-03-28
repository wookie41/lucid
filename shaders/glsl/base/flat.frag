#version 330 core

uniform vec4 uMaterialColor;
out vec4 oFragColor;

void main()
{
    oFragColor = uMaterialColor;
}