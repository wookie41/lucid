uniform int uMeshBatchOffset;

struct FActorData
{
    mat4 ModelMatrix;
    int  NormalMultiplier;
    uint ActorId;
};

struct FInstanceData
{
    int ActorDataIdx;
    int MaterialDataIdx;
};

layout(std430, binding = 1) buffer ActorDataBlock { FActorData ActorData[]; };
layout(std430, binding = 2) buffer InstanceDataBlock { FInstanceData InstanceData[]; };

#define INSTANCE_DATA ActorData[InstanceData[uMeshBatchOffset + InstanceID].ActorDataIdx]
#define MATERIAL_DATA_INDEX InstanceData[uMeshBatchOffset + InstanceID].MaterialDataIdx
#define MATERIAL_DATA MaterialData[MATERIAL_DATA_INDEX]