#version 330 core

in vec4 FragPos;

uniform vec3 uLightPosition;
uniform float uLightFarPlane;

void main()
{
    float distanceToLight = length(FragPos.xyz - uLightPosition);
    gl_FragDepth = distanceToLight / uLightFarPlane;
}