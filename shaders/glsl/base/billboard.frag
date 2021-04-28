#version 330 core

in vec2 oTextureCoords;

uniform sampler2D   uBillboardTexture;
uniform vec3        uBillboardColorTint;

out vec4 oFragColor;

void main()
{
    oFragColor = texture(uBillboardTexture, oTextureCoords) * vec4(uBillboardColorTint, 1);
}