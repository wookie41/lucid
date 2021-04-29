const vec3 TAN_SPACE_UP = vec3( 0, 0, 1 );

uniform float uParallaxHeightScale;

vec2 ParallaxOcclusionMapping(in vec3 TanToViewN, in vec2 TexCoords, in sampler2D DisplacementMap)
{
    float numOfLayers = mix(32, 8, abs(dot(TAN_SPACE_UP, TanToViewN)));
    float layerDepth = 1.0/numOfLayers;
    float currentLayerDepth = 0;
    vec2 p = TanToViewN.xy / TanToViewN.z * uParallaxHeightScale;
    vec2 deltaTexCoords = p / numOfLayers;
    vec2 texCoords = TexCoords;
    float currentMapValueDepth = texture(DisplacementMap, texCoords).r; 

    while (currentMapValueDepth > currentLayerDepth)
    {
        texCoords -= deltaTexCoords;
        if (texCoords.x < 0 || texCoords.x > 1 || texCoords.y < 0 || texCoords.y > 1)
        {
            return texCoords;
        }
        currentLayerDepth += layerDepth;
        currentMapValueDepth = texture(DisplacementMap, texCoords).r;
    }

    vec2 prevTexCoords = texCoords + deltaTexCoords;

    float afterDepth  = currentMapValueDepth - currentLayerDepth;
    float beforeDepth = texture(DisplacementMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + texCoords * (1.0 - weight);

    return finalTexCoords;
}