#version 330 core


uniform uint uRenderableId;

out uint oRenderableId;

void main() 
{
    oRenderableId = uRenderableId;
}