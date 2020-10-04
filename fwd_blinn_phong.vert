#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoords;

out vec3 Normal;
out vec3 WorldPos;
out vec2 TextureCoods;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    Normal = transpose(inverse(mat3(uModel))) * aNormal;
    TextureCoods = aTextureCoords;

    vec4 worldPos = uModel * vec4(aPosition, 1);
    WorldPos = worldPos.xyz;

    gl_Position = uProjection * uView * worldPos;
}
