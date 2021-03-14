#version 330 core

#include "material.glsl"
#include "parallax_occlusion.glsl"

layout (location = 0) out vec3 oNormalVS;
layout (location = 1) out vec3 oPositionVS;

in vec3 PositionVS;
in vec3 NormalVS;
in vec2 TexCoords;
flat in mat3 TBNMatrix;

uniform BlinnPhongMapsMaterial uMaterial;
uniform vec3 uViewPos;

void main() 
{
    vec2 textureCoords = TexCoords;

    if (uMaterial.HasNormalMap)
    {
        if (uMaterial.HasDisplacementMap)
        {
            vec3 toViewN = normalize(-PositionVS);
            textureCoords = ParallaxOcclusionMapping(inverse(TBNMatrix) * toViewN, textureCoords, uMaterial.DisplacementMap);
            if (textureCoords.x > 1 || textureCoords.x < 0 || textureCoords.y > 1 || textureCoords.y < 0)
            {
                discard;
            }
        }
        
        vec3 SampledNormal = (texture(uMaterial.NormalMap, textureCoords).rgb * 2) - 1;
        oNormalVS = normalize(TBNMatrix * SampledNormal);
    }
    else
    {
        oNormalVS = normalize(NormalVS);
    }
    oPositionVS = PositionVS;
}
