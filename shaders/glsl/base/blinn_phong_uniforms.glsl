flat in int InstanceID;

struct FBlinnPhongMaterial
{
    vec3 DiffuseColor;
    int  Shininess;
    vec3 SpecularColor;
};

layout(std430, binding = 3) buffer MaterialDataDataBlock { FBlinnPhongMaterial MaterialData[]; };

