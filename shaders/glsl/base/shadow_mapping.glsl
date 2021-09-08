float CalculateShadow(in vec3 FragPos, in vec3 NormalN, in vec3 LightDirN)
{
    if (!uLightCastsShadows)
    {
        return 1;
    }

    sampler2D ShadowMap = uLightShadowMap;
    vec4 lightSpaceFragPos = vec4(0);

    if (uLightType == DIRECTIONAL_LIGHT)
    {
        float FragViewSpaceZ = (uView * vec4(FragPos, 1.0)).z;

        int CascadeIndex = 0;
        while (FragViewSpaceZ > uCascadeFarPlanes[CascadeIndex] && CascadeIndex < uCascadeCount)
        {
            CascadeIndex += 1;
        }

        lightSpaceFragPos = uCascadeMatrices[CascadeIndex] * vec4(FragPos, 1.0);
        ShadowMap = uCascadeShadowMaps[CascadeIndex];
    }
    else
    {
        lightSpaceFragPos = uLightMatrix * vec4(FragPos, 1.0);
    }

    vec3 clipSpaceCoords   = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    clipSpaceCoords        = (clipSpaceCoords * 0.5) + 0.5;

    if (clipSpaceCoords.z > 1.0)
    {
        return 1;
    }

    float currentDepth = clipSpaceCoords.z;    

    float samplesSum = 0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    int numSamples = uNumPCFSamples / 2;
    float bias = max(0.05 * (1.0 - dot(NormalN, normalize(uLightPosition - FragPos))), 0.005);
    currentDepth -= bias;
    if (uNumPCFSamples == 1)
    {
        float closestDepth = texture(ShadowMap, clipSpaceCoords.xy).r;
        return currentDepth > closestDepth ? 0.0 : 1.0;
    }
    for (int x = -numSamples; x <= numSamples; ++x)
    {
        for (int y = -numSamples; y <= numSamples; ++y)
        {
            float closestDepth = texture(ShadowMap, clipSpaceCoords.xy + vec2(x, y) * texelSize).r;
            samplesSum += (currentDepth > closestDepth ? 0.0 : 1.0);
        }
    }
    return samplesSum / (uNumPCFSamples * uNumPCFSamples);
}

vec3 pcfDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float CalculateShadowCubemap(in vec3 FragPos, in vec3 NormalN, in vec3 LightPos)
{
    if (!uLightCastsShadows)
    {
        return 1.0;
    }

    vec3 toFrag = FragPos - LightPos;
    float distanceToLight = length(toFrag);
    float bias = max(0.05 * (1.0 - dot(NormalN, normalize(toFrag))), 0.005);
    float currentDepth = distanceToLight - bias;
    float shadow = 0;
    float numOfSamples = min(uNumPCFSamples, 20);
    float viewDistance = length(uViewPos - FragPos);
    float diskRadius = (1.0 + (viewDistance / uLightFarPlane)) / uLightFarPlane;
    
    for (int i = 0; i < numOfSamples; i++)
    {
        vec3 ShadowMapCoords = normalize(toFrag + (pcfDirections[i] * diskRadius));
        float closestDepth = texture(uLightShadowCube, ShadowMapCoords).r;
        closestDepth *= uLightFarPlane;
        shadow += currentDepth > closestDepth ? 0.0 : 1.0;
    }

    return shadow / float(numOfSamples);
}
