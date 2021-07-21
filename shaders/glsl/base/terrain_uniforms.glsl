layout(bindless_sampler) uniform;

#define MAX_TERRAIN_LAYERS 8

struct FTerrainLayer
{
    sampler2D DiffuseMap;
    sampler2D NormalMap;
    vec2      UVTiling;
    float     MaxHeight;
    bool      bHasDiffuseMap;
    bool      bHasNormalMap;
};

struct FTerrainMaterial
{
    FTerrainLayer Layers[MAX_TERRAIN_LAYERS];
    int           NumLayers;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FTerrainMaterial MaterialData[]; };