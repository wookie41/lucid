#version 330

layout (location = 0) in vec3 aPosition;
layout (location = 3) in vec2 aTextureCoords;

out vec2 TextureCoords;

void main() 
{
    TextureCoords = aTextureCoords;
    gl_Position = vec4(aPosition, 1.0);
}
