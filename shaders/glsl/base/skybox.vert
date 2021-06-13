#version 450 core

layout(location = 0) in vec3 aPosition;
 
out vec3 iTextureCoords;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    iTextureCoords = aPosition;
    gl_Position = (uProjection * uView * vec4(aPosition, 0.0)).xyww;
}