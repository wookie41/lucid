#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"
#include "terrain_uniforms.glsl"

in int       gl_InstanceID;
flat out int InstanceID;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aTextureCoords;

out VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
}
vsOut;

void main()
{
    InstanceID = gl_InstanceID;

    vec4 WorldPosition = vec4(aPosition, 1);
    if (MATERIAL_DATA.bHasDeltaHeightMap)
    {
        WorldPosition.y += texture(MATERIAL_DATA.DeltaHeightMap, aTextureCoords).r;
    }

    mat3 NormalMatrix = transpose(inverse(mat3(INSTANCE_DATA.ModelMatrix)));
    vec3 N            = normalize(NormalMatrix * aNormal);
    WorldPosition     = INSTANCE_DATA.ModelMatrix * WorldPosition;

    vsOut.FragPos            = WorldPosition.xyz;
    vsOut.TextureCoords      = aTextureCoords;
    vsOut.InterpolatedNormal = N;

    gl_Position = uProjection * uView * WorldPosition;
}
