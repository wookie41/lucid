#version 330
in vec2 TextureCoords;

uniform sampler2D uTextureToBlur;
uniform int uOffsetX;
uniform int uOffsetY;

out float BlurredPixel;

void main() {
    vec2 TexelSize = 1.0 / vec2(textureSize(uTextureToBlur, 0));
    
    float AccumulatedColor;
    for (int y = -uOffsetY; y < uOffsetY; ++y)
    {
        for (int x = -uOffsetX; x < uOffsetX; ++x)
        {
            AccumulatedColor += texture(uTextureToBlur, TextureCoords + vec2(TexelSize.x * x, TexelSize.y * y)).r;
        }
    }

    BlurredPixel = AccumulatedColor / (uOffsetX * 2 * uOffsetY * 2);
}
