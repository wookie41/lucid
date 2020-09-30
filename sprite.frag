#version 330 core

layout(location = 0) out vec4 FragColor;

in vec2 TextureCoords;
uniform sampler2D SpriteTexture;

void main() { FragColor = vec4( texture(SpriteTexture, TextureCoords).rgb,1.0); }