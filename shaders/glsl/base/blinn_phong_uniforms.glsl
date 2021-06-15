flat in int InstanceID;

struct FBlinnPhongMaterial
{
    vec3 DiffuseColor;
    vec3 SpecularColor;
    int  Shininess;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FBlinnPhongMaterial MaterialData[]; };

#define MATERIAL_DATA MaterialData[InstanceID]
