#version 450 core

#include "forward_prepass_common.glsl"
#include "parallax_occlusion.glsl"

flat in int InstanceID;

layout (location = 0) out vec3 oNormalVS;
layout (location = 1) out vec3 oPositionVS;

in vec3 PositionVS;
in vec3 NormalVS;
in vec2 TexCoords;

flat in mat3 TBNMatrix;
uniform vec3 uViewPos;

void main() 
{
    vec2 textureCoords = TexCoords;

    if (PREPASS_DATA.bHasNormalMap)
    {
        if (PREPASS_DATA.bHasDisplacementMap)
        {
            vec3 toViewN = normalize(-PositionVS);
            textureCoords = ParallaxOcclusionMapping(inverse(TBNMatrix) * toViewN, textureCoords, PREPASS_DATA.DisplacementMap);
            if (textureCoords.x > 1 || textureCoords.x < 0 || textureCoords.y > 1 || textureCoords.y < 0)
            {
                discard;
            }
        }
        
        vec3 SampledNormal = (texture(PREPASS_DATA.NormalMap, textureCoords).rgb * 2) - 1;
        oNormalVS = normalize(TBNMatrix * SampledNormal);
    }
    else
    {
        oNormalVS = normalize(NormalVS);
    }
    oPositionVS = PositionVS;
}
