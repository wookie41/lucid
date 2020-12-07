float CalculateShadow(in vec3 FragPos, in vec3 NormalN, in vec3 LightDirN)
{
    if (!uLight.CastsShadows) 
    {
        return 1.0;
    }

    vec4 lightSpaceFragPos = uLight.LightSpaceMatrix * vec4(FragPos, 1.0);
    vec3 clipSpaceCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    clipSpaceCoords = (clipSpaceCoords * 0.5) + 0.5;
    if (clipSpaceCoords.z > 1.0) {
        return 1.0;
    }
    float closestDepth = texture(uLight.ShadowMap, clipSpaceCoords.xy).r;
    float currentDepth = clipSpaceCoords.z;
    float bias = max(0.05 * (1.0 - dot(NormalN, normalize(uLight.Position - FragPos))), 0.005);  
    return (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
}
