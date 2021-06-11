#version 420 core

in vec2 inTextureCoords;

out float oFragColor;

uniform sampler2D uPositionsVS;
uniform sampler2D uNormalsVS;
uniform sampler2D uNoise;

uniform vec3 uSamples[64];
uniform vec2 uNoiseScale;

uniform float uRadius;
uniform float uBias;

uniform mat4 uProjection;

void main()
{
    // Fetch position and normal in viewspace prepared by the ealier pass
    vec3 FragPosVS = texture(uPositionsVS, inTextureCoords).xyz;
    vec3 NormalVS = normalize(texture(uNormalsVS, inTextureCoords).rgb);
    vec3 RandomVec = vec3(normalize(texture(uNoise, inTextureCoords * uNoiseScale).xy), 0);
    
    // Build a TBN matrix that we'll use to transfrom he samples from tangent space to view space
    vec3 T = normalize(RandomVec - NormalVS * dot(RandomVec, NormalVS));
    vec3 B = cross(NormalVS, T);
    mat3 TBN = mat3(T, B, NormalVS);
    
    // Calculate occlusion factor
    float Occlusion = 0;
    for (int i = 0; i < 64; ++i)
    {
        // Fetch the sample, transform it to view space and add it to current fragment's position
        vec3 SampleVS = TBN * uSamples[i];
        vec3 SamplePosVS = FragPosVS + (SampleVS * uRadius);
        
        // Project the sample to NDC so we can read the depth at sample's position 
        vec4 Offset = uProjection * vec4(SamplePosVS, 1.0);
        Offset.xyz /= Offset.w;
        Offset.xyz = (Offset.xyz * 0.5) + 0.5;
        
        // Get depth at sample's position
        float SampledDepth = texture(uPositionsVS, Offset.xy).z;
        float RangeCheck = smoothstep(0.0, 1.0, uRadius / abs(FragPosVS.z - SampledDepth));

        // Compare depth at sample's position and sample's depth
        if (SampledDepth >= SamplePosVS.z + uBias)
        {
            Occlusion += RangeCheck;
        }
    }

    oFragColor = 1.0 - (Occlusion / 64.0);
}