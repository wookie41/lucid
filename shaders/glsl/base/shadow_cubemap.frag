#version 330 core

in vec4 FragPos;

uniform vec3 uLightPosition;
uniform float uLightFarPlane;

void main()
{
    float distanceToLight = length(uLightPosition - FragPos.xyz);
    gl_FragDepth = distanceToLight / uLightFarPlane;
}