#version 330 core

in vec4 FragPos;

uniform vec3 uLightPosition;
uniform float uFarPlane;

void main()
{
    float distanceToLight = length(uLightPosition - FragPos.xyz);
    gl_FragDepth = distanceToLight / uFarPlane;
}