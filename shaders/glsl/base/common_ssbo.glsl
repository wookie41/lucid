#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable

uniform int uMeshBatchOffset;

struct FCommonInstanceData
{
    mat4 ModelMatrix;
    int  NormalMultiplier;
};

layout(std430, binding = 0) buffer ModelMatricesDataBlock { FCommonInstanceData CommonInstanceData[]; };

#define INSTANCE_DATA CommonInstanceData[uMeshBatchOffset + InstanceID]
