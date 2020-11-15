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
    sampler2D SpecularMap;
    sampler2D NormalMap;

    bool HasSpecularMap;
    bool HasNormalMap;
};