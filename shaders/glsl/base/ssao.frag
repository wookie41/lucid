#version 330 core

out vec3 oFragColor;

uniform sampler2D uPositionsVS;
uniform sampler2D uNormalsVS;
uniform sampler2D uNoise;

uniform vec2 uSamples[64];

void main()
{
    oFragColor = vec3(1, 1, 0.33);
}