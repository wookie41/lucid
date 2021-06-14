#extension GL_ARB_shader_storage_buffer_object : enable

uniform int uMeshBatchOffset;

struct FCommonInstanceData
{
    mat4 ModelMatrix;
    int  NormalMultiplier;
};

layout(std430, binding = 0) buffer CommonInstanceDataBlock { FCommonInstanceData CommonInstanceData[]; };

#define INSTANCE_DATA CommonInstanceData[uMeshBatchOffset + InstanceID]
