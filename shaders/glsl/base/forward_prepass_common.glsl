#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#include "common_ssbo.glsl"

struct FForwardPrepassUniforms
{
    bool bHasNormalMap;
    bool bHasDisplacementMap;
};

layout(std430, binding = 1) buffer PrepassDataBlock 
{ 
    FForwardPrepassUniforms PrepassData[]; 
};

layout(bindless_sampler) uniform;

layout(binding = 2) buffer PrepassTexturesBlock 
{
    sampler2D PrepassTextures[];
};

#define PREPASS_DATA PrepassData[uMeshBatchOffset + InstanceID]
#define NORMAL_MAP PrepassTextures[(uMeshBatchOffset + InstanceID) * 2]
#define DISPLACEMENT_MAP PrepassTextures[((uMeshBatchOffset + InstanceID) * 2) + 1]
