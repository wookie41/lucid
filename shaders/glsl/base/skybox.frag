#version 420 core

in vec3 iTextureCoords;

uniform samplerCube uSkybox;

out vec4 oFragColor;

void main() 
{
    oFragColor = texture(uSkybox, iTextureCoords);
}