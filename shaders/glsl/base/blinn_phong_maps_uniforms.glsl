
flat in int InstanceID;

layout(bindless_sampler) uniform;

struct FBlinnPhongMapsMaterial
{
    vec3 SpecularColor;

    sampler2D DiffuseMap;
    sampler2D SpecularMap;
    sampler2D NormalMap;
    sampler2D DisplacementMap;

    int  Shininess;
    bool bHasSpecularMap;
    bool bHasNormalMap;
    bool bHasDisplacementMap;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FBlinnPhongMapsMaterial MaterialData[]; };