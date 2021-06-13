#version 450 core

#define WORLD_SPACE 0
#define VIEW_SPACE 1

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in int aSpaceType;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 inColor;

void main() 
{
    inColor = aColor;
    if (aSpaceType == WORLD_SPACE)
    {
        gl_Position = uProjection * uView * vec4(aPos, 1);
    }
    else if (aSpaceType == VIEW_SPACE)
    {
        gl_Position = uProjection * vec4(aPos, 1);
    }
    else
    {
        gl_Position = vec4(aPos, 1);
    }
}
