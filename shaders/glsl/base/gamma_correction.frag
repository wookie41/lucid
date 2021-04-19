#version 330 core

in vec2 inTextureCoords;

uniform float uGamma;
uniform sampler2D uSceneTexture;

out vec4 oFragColor;

void main()
{
    vec4 fragColor = texture(uSceneTexture, inTextureCoords);
    oFragColor = vec4(pow(fragColor.rgb, vec3(1.0/uGamma)), fragColor.a);
}