#include "batch_instance.glsl"

layout(bindless_sampler) uniform;

struct FForwardPrepassUniforms
{
    bool bHasNormalMap;
    bool bHasDisplacementMap;
    
    sampler2D NormalMap;
    sampler2D DisplacementMap;
};

layout(std430, binding = 3) buffer PrepassDataBlock 
{ 
    FForwardPrepassUniforms PrepassData[]; 
};

#define PREPASS_DATA PrepassData[uMeshBatchOffset + InstanceID]
