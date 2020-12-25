struct BlinnPhongMaterial
{
    int Shininess;
    vec3 DiffuseColor;
    vec3 SpecularColor;
};

struct BlinnPhongMapsMaterial
{
    int Shininess;

    sampler2D DiffuseMap;
    
    bool HasSpecularMap;
    sampler2D SpecularMap;
    vec3 SpecularColor;

    bool HasNormalMap;
    sampler2D NormalMap;

    bool HasDisplacementMap;
    sampler2D DisplacementMap;
};


struct FlatMaterial
{
    vec4 Color;
};