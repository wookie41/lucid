#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#include "common_ssbo.glsl"

layout(bindless_sampler) uniform;

struct FForwardPrepassUniforms
{
    bool bHasNormalMap;
    bool bHasDisplacementMap;
    
    sampler2D NormalMap;
    sampler2D DisplacementMap;
};

layout(std430, binding = 1) buffer PrepassDataBlock 
{ 
    FForwardPrepassUniforms PrepassData[]; 
};

#define PREPASS_DATA PrepassData[uMeshBatchOffset + InstanceID]
