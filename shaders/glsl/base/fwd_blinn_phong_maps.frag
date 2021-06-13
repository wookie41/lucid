#version 450 core

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

uniform float       uAmbientStrength;
uniform sampler2D   uAmbientOcclusion;

uniform vec3    uViewPos;
uniform vec2    uViewportSize;

uniform mat4    uModel;

#include "lights.glsl"
#include "shadow_mapping.glsl"
#include "parallax_occlusion.glsl"

out vec4 oFragColor;

void main()
{
    vec3 toViewN = normalize(uViewPos - fsIn.FragPos);
    vec2 ScreenSpaceCoords = (gl_FragCoord.xy / uViewportSize);
    float AmbientOcclusion = texture(uAmbientOcclusion, ScreenSpaceCoords).r;

    vec2 textureCoords = fsIn.TextureCoords;
    if (uMaterialHasDisplacementMap)
    {
        textureCoords = ParallaxOcclusionMapping(fsIn.inverseTBN * toViewN, fsIn.TextureCoords, uMaterialDisplacementMap);
        if (textureCoords.x > 1 || textureCoords.x < 0 || 
            textureCoords.y > 1 || textureCoords.y < 0)
        {
            discard;
        }
    }
    
    vec3 normal;
    if (uMaterialHasNormalMap)
    {
        normal = normalize(fsIn.TBN * ((texture(uMaterialNormalMap, textureCoords).rgb * 2) - 1));
    }
    else
    {
        normal = normalize(fsIn.InterpolatedNormal);
    }

    vec3 diffuseColor =  texture(uMaterialDiffuseMap, textureCoords).rgb * AmbientOcclusion;
    vec3 specularColor = uMaterialHasSpecularMap ? texture(uMaterialSpecularMap, textureCoords).rgb : uMaterialSpecularColor; 

    vec3 ambient = diffuseColor * uAmbientStrength * AmbientOcclusion;
    
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLightType == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterialShininess);
    }
    else if (uLightType == POINT_LIGHT)
    {   
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLightPosition);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterialShininess);
    }
    else if (uLightType == SPOT_LIGHT)
    {
        shadowFactor     = CalculateShadow(fsIn.FragPos, normal, normalize(uLightDirection));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterialShininess);
    }
    
    vec3 fragColor = (diffuseColor * lightCntrb.Diffuse) + specularColor * lightCntrb.Specular;
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 1);
}

