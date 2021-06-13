#version 450 core

in vec3 Normal;
out vec4 oFragColor;

const vec3 ToFakeLightDir = vec3(-2.5, 1.5, 0.5);

void main()
{
    oFragColor = dot(Normal, normalize(ToFakeLightDir)) * vec4(0.65);
}