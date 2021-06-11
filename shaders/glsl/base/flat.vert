#version 420 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vec4 WorldPos = uModel * vec4(aPosition, 1.0);
    gl_Position = uProjection * uView * WorldPos;
}
