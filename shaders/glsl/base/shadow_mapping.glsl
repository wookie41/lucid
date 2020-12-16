uniform int uNumSamplesPCF;

float CalculateShadow(in vec3 FragPos, in vec3 NormalN, in vec3 LightDirN)
{
    if (!uLight.CastsShadows)
    {
        return 1.0;
    }

    vec4 lightSpaceFragPos = uLight.LightSpaceMatrix * vec4(FragPos, 1.0);
    vec3 clipSpaceCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    clipSpaceCoords = (clipSpaceCoords * 0.5) + 0.5;
    if (clipSpaceCoords.z > 1.0)
    {
        return 1.0;
    }
    float currentDepth = clipSpaceCoords.z;
    float samplesSum = 0;
    vec2 texelSize = 1.0 / textureSize(uLight.ShadowMap, 0);
    int numSamples = uNumSamplesPCF / 2;
    float bias = max(0.05 * (1.0 - dot(NormalN, normalize(uLight.Position - FragPos))), 0.005);
    currentDepth -= bias;
    if (uNumSamplesPCF == 1)
    {
        float closestDepth = texture(uLight.ShadowMap, clipSpaceCoords.xy).r;
        return currentDepth > closestDepth ? 0.0 : 1.0;
    }
    for (int x = -numSamples; x <= numSamples; ++x)
    {
        for (int y = -numSamples; y <= numSamples; ++y)
        {
            float closestDepth = texture(uLight.ShadowMap, clipSpaceCoords.xy + vec2(x, y) * texelSize).r;
            samplesSum += (currentDepth > closestDepth ? 0.0 : 1.0);
        }
    }
    return samplesSum / (uNumSamplesPCF * uNumSamplesPCF);
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
    if (!uLight.CastsShadows)
    {
        return 1.0;
    }

    vec3 toFrag = FragPos - LightPos;
    float distanceToLight = length(toFrag);
    float viewDistance = length(uViewPos - FragPos);
    float diskRadius = (1.0 + (viewDistance / uLight.FarPlane)) / uLight.FarPlane;
    float bias = max(0.05 * (1.0 - dot(NormalN, normalize(toFrag))), 0.005);
    float currentDepth = (distanceToLight / uLight.FarPlane) - bias;
    float shadow = 0;
    float numOfSamples = min(uNumSamplesPCF, 20);
    
    for (int i = 0; i < numOfSamples; i++)
    {
        float closestDepth = texture(uShadowCube, toFrag + (pcfDirections[i] * diskRadius)).r;
        shadow += currentDepth > closestDepth ? 0.0 : 1.0;
    }

    return shadow / float(numOfSamples);
}
