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
