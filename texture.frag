#version 330 core

layout(location = 0) out vec4 FragColor;

in vec2 TextureCoords;
uniform sampler2D SpriteTexture;

void main() { FragColor = texture(SpriteTexture, TextureCoords); }