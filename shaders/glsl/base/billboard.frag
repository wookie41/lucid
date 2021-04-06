#version 330 core

in vec2 oTextureCoords;

uniform sampler2D uBillboardTexture;

out vec4 oFragColor;

void main()
{
    oFragColor = texture(uBillboardTexture, oTextureCoords);
}