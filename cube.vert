#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTextureCoords;

out vec2 TextureCoords;

uniform mat4 View;
uniform mat4 Projection; 

void main()
{
    TextureCoords = aTextureCoords;
    gl_Position =  Projection * View * vec4(aPosition, 1);
}