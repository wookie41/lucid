#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable


struct FForwardPrepassUniforms
{
    mat4 ModelMatrix;
    int  NormalMultiplier;
    bool bHasNormalMap;
    bool bHasDisplacementMap;
};

uniform int uPrepassDataBlockOffset;

layout(std430, binding = 0) buffer PrepassDataBlock 
{ 
    FForwardPrepassUniforms PrepassData[]; 
};

layout(bindless_sampler) uniform;

layout(std430, binding = 1) buffer PrepassTexturesBlock 
{
    sampler2D PrepassTextures[];
};

#define PREPASS_DATA PrepassData[uPrepassDataBlockOffset + InstanceId]
#define NORMAL_MAP PrepassTextures[(uPrepassDataBlockOffset + InstanceId) * 2]
#define DISPLACEMENT_MAP PrepassTextures[((uPrepassDataBlockOffset + InstanceId) * 2) + 1]
