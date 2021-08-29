#version 450 core

#include "common.glsl"
#include "batch_instance.glsl"
#include "blinn_phong_maps_uniforms.glsl"

in VS_OUT
{
    vec3 FragPos;
    vec2 TextureCoords;
    vec3 InterpolatedNormal;
    mat3 TBN;
    mat3 inverseTBN;
}
fsIn;

in vec4 gl_FragCoord;

uniform mat4 uModel;

#include "lights.glsl"
#include "shadow_mapping.glsl"
#include "parallax_occlusion.glsl"

out vec4 oFragColor;

void main()
{
    vec3  toViewN           = normalize(uViewPos - fsIn.FragPos);
    vec2  ScreenSpaceCoords = (gl_FragCoord.xy / uViewportSize);
    float AmbientOcclusion  = texture(uAmbientOcclusion, ScreenSpaceCoords).r;

    vec2 textureCoords = fsIn.TextureCoords;
    if (MATERIAL_DATA.bHasDisplacementMap)
    {
        textureCoords = ParallaxOcclusionMapping(fsIn.inverseTBN * toViewN, fsIn.TextureCoords, MATERIAL_DATA.DisplacementMap);
        if (textureCoords.x > 1 || textureCoords.x < 0 || textureCoords.y > 1 || textureCoords.y < 0)
        {
            discard;
        }
    }
    
    vec3 normal;
    if (MATERIAL_DATA.bHasNormalMap)
    {
        normal = normalize(fsIn.TBN * ((texture(MATERIAL_DATA.NormalMap, textureCoords).rgb * 2) - 1));
    }
    else
    {
        normal = normalize(fsIn.InterpolatedNormal);
    }

    vec3 diffuseColor  = texture(MATERIAL_DATA.DiffuseMap, textureCoords).rgb * AmbientOcclusion;
    vec3 specularColor = MATERIAL_DATA.bHasSpecularMap ? texture(MATERIAL_DATA.SpecularMap, textureCoords).rgb : MATERIAL_DATA.SpecularColor;

    vec3 ambient = diffuseColor * uAmbientStrength * AmbientOcclusion;

    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb   = CalculateDirectionalLightContribution(toViewN, normal, MATERIAL_DATA.Shininess);
    }
    else if (uLightType == POINT_LIGHT)
    {
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLightPosition);
        lightCntrb   = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, MATERIAL_DATA.Shininess);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb   = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, MATERIAL_DATA.Shininess);
    }

    vec3 fragColor = (diffuseColor * lightCntrb.Diffuse) + (specularColor * lightCntrb.Specular);
    
    fragColor *= lightCntrb.Attenuation * uLightIntensity * shadowFactor;
    ambient *= lightCntrb.Diffuse;
    
    oFragColor     = vec4(ambient + fragColor, 1);
}
