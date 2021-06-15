#version 450 core

#include "common.glsl"

layout(location = 0) in vec3 aPosition;
 
out vec3 iTextureCoords;

void main()
{
    iTextureCoords = aPosition;
    gl_Position = (uProjection * uView * vec4(aPosition, 0.0)).xyww;
}