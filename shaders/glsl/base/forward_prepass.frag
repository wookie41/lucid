#version 330 core

#include "material.glsl"

layout (location = 0) out vec3 oNormalVS;
layout (location = 1) out vec3 oPositionVS;

in vec3 PositionVS;
in vec3 NormalVS;
in vec2 TexCoords;
in mat3 TBNMatrix;

uniform BlinnPhongMapsMaterial uMaterial;

void main() 
{
    // @TODO add displacment map support for texture coords
    if (uMaterial.HasNormalMap)
    {
        vec3 SampledNormal = (texture(uMaterial.NormalMap, TexCoords).rgb * 2) - 1;
        oNormalVS = normalize(TBNMatrix * SampledNormal);
    }
    else
    {
        oNormalVS = normalize(NormalVS);
    }

    oPositionVS = PositionVS;
}
