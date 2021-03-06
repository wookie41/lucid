#version 330 core

#include "material.glsl"


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

uniform float uAmbientStrength;
uniform vec3 uViewPos;
uniform BlinnPhongMapsMaterial uMaterial;
uniform mat4 uModel;
uniform sampler2D uAmbientOcclusion;
uniform vec2 uViewportSize;

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
    if (uMaterial.HasDisplacementMap)
    {
        textureCoords = ParallaxOcclusionMapping(fsIn.inverseTBN * toViewN, fsIn.TextureCoords, uMaterial.DisplacementMap);
        if (textureCoords.x > 1 || textureCoords.x < 0 || 
            textureCoords.y > 1 || textureCoords.y < 0)
        {
            discard;
        }
    }

    vec3 normal;
    if (uMaterial.HasNormalMap)
    {
        normal = normalize(fsIn.TBN * ((texture(uMaterial.NormalMap, textureCoords).rgb * 2) - 1));
    }
    else
    {
        normal = normalize(fsIn.InterpolatedNormal);
    }

    vec3 diffuseColor =  texture(uMaterial.DiffuseMap, textureCoords).rgb * AmbientOcclusion;
    vec3 specularColor = uMaterial.HasSpecularMap ? texture(uMaterial.SpecularMap, textureCoords).rgb : uMaterial.SpecularColor; 

    vec3 ambient = diffuseColor * uAmbientStrength * AmbientOcclusion;
    float shadowFactor = 1.0;

    LightContribution lightCntrb;
    if (uLight.Type == DIRECTIONAL_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculateDirectionalLightContribution(toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == POINT_LIGHT)
    {   
        shadowFactor = CalculateShadowCubemap(fsIn.FragPos, normal, uLight.Position);
        lightCntrb = CalculatePointLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }
    else if (uLight.Type == SPOT_LIGHT)
    {
        shadowFactor = CalculateShadow(fsIn.FragPos, normal, normalize(uLight.Direction));
        lightCntrb = CalculateSpotLightContribution(fsIn.FragPos, toViewN, normal, uMaterial.Shininess);
    }
    
    vec3 fragColor = (diffuseColor * lightCntrb.Diffuse) + (specularColor * lightCntrb.Specular);
    oFragColor = vec4((ambient * lightCntrb.Attenuation) + (fragColor * shadowFactor), 0);
}
