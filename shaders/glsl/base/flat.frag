#version 450 core

#include "batch_instance.glsl"

flat in int InstanceID;

struct FFlatMaterial
{
    vec4 Color;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FFlatMaterial MaterialData[]; };


out vec4 oFragColor;
    
void main() { oFragColor = MATERIAL_DATA.Color; }