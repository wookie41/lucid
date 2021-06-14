#version 450 core

#extension GL_ARB_shader_storage_buffer_object : enable

flat in int InstanceID;

struct FFlatMaterial
{
    vec4 Color;
};

layout(std430, binding = 1) buffer MaterialDataDataBlock { FFlatMaterial MaterialData[]; };

#define MATERIAL_DATA MaterialData[InstanceID]

out vec4 oFragColor;

void main() { oFragColor = MATERIAL_DATA.Color; }