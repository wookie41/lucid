uniform int uMeshBatchOffset;

struct FCommonInstanceData
{
    mat4 ModelMatrix;
    int  NormalMultiplier;
};

layout(std430, binding = 1) buffer CommonInstanceDataBlock { FCommonInstanceData CommonInstanceData[]; };

#define INSTANCE_DATA CommonInstanceData[uMeshBatchOffset + InstanceID]
