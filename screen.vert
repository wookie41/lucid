#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 textureCoords;

out vec2 TextureCoords;

uniform vec2 Position;
// uniform float Rotation; //TODO Use it
uniform ivec2 SpriteSize;
uniform ivec2 TextureRegionSize;
uniform ivec2 TextureRegionIndex;
uniform ivec2 TextureSize;

uniform mat4 View;
uniform mat4 Projection;

void main()
{
    vec2 normalizedPos = (position + 1.0) / 2.0;
    vec2 screenPosition = vec2(Position + (SpriteSize * normalizedPos));

    TextureCoords = (vec2(SpriteSize * TextureRegionIndex) + vec2(textureCoords * TextureRegionSize)) / TextureSize;
    gl_Position = Projection * View * vec4(screenPosition, -1, 1);
}