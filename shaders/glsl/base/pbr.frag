#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"
#include "pbr_lights.glsl"

flat in int InstanceID;

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
    mat3 inverseTBN;
}
fsIn;

struct FPBRMaterial
{
    vec3  Albedo;
    float Metallic;
    float Roughness;
};

#include "pbr.glsl"

layout(std430, binding = 3) buffer MaterialDataDataBlock { FPBRMaterial MaterialData[]; };

out vec4 oFragColor;

void main() { oFragColor = vec4(CalculatePBR(normalize(fsIn.InterpolatedNormal), MATERIAL_DATA.Roughness, MATERIAL_DATA.Metallic, MATERIAL_DATA.Albedo), 1.0); }