uniform int uMeshBatchOffset;

struct FActorData
{
    mat4 ModelMatrix;
    int  NormalMultiplier;
};

struct FInstanceData
{
    int ActorDataIdx;
};

layout(std430, binding = 1) buffer ActorDataBlock { FActorData ActorData[]; };
layout(std430, binding = 2) buffer InstanceDataBlock { FInstanceData InstanceData[]; };

#define INSTANCE_DATA ActorData[InstanceData[uMeshBatchOffset + InstanceID].ActorDataIdx]
