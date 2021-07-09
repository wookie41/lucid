layout(bindless_sampler) uniform;

struct FTerrainMaterial
{
    sampler2D DeltaHeightMap;
    bool      bHasDeltaHeightMap;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FTerrainMaterial MaterialData[]; };