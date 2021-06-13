#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#include "model_matrices_ssbo.glsl"

struct FForwardPrepassUniforms
{
    int  NormalMultiplier;
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

#define PREPASS_DATA PrepassData[uMeshBatchOffset + InstanceId]
#define NORMAL_MAP PrepassTextures[(uMeshBatchOffset + InstanceId) * 2]
#define DISPLACEMENT_MAP PrepassTextures[((uMeshBatchOffset + InstanceId) * 2) + 1]
