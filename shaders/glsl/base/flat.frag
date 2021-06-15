#version 450 core

flat in int InstanceID;

struct FFlatMaterial
{
    vec4 Color;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FFlatMaterial MaterialData[]; };

#define MATERIAL_DATA MaterialData[InstanceID]

out vec4 oFragColor;
    
void main() { oFragColor = MATERIAL_DATA.Color; }