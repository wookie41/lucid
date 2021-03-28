#version 330 core

#include "parallax_occlusion.glsl"

layout (location = 0) out vec3 oNormalVS;
layout (location = 1) out vec3 oPositionVS;

in vec3 PositionVS;
in vec3 NormalVS;
in vec2 TexCoords;
flat in mat3 TBNMatrix;

uniform vec3 uViewPos;

uniform bool        uMaterialHasNormalMap;
uniform sampler2D   uMaterialNormalMap;

uniform bool        uMaterialHasDisplacementMap;
uniform sampler2D   uMaterialDisplacementMap;

void main() 
{
    vec2 textureCoords = TexCoords;

    if (uMaterialHasNormalMap)
    {
        if (uMaterialHasDisplacementMap)
        {
            vec3 toViewN = normalize(-PositionVS);
            textureCoords = ParallaxOcclusionMapping(inverse(TBNMatrix) * toViewN, textureCoords, uMaterialDisplacementMap);
            if (textureCoords.x > 1 || textureCoords.x < 0 || textureCoords.y > 1 || textureCoords.y < 0)
            {
                discard;
            }
        }
        
        vec3 SampledNormal = (texture(uMaterialNormalMap, textureCoords).rgb * 2) - 1;
        oNormalVS = normalize(TBNMatrix * SampledNormal);
    }
    else
    {
        oNormalVS = normalize(NormalVS);
    }
    oPositionVS = PositionVS;
}
