#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoords;

out vec3 iNormal;
out vec3 iWorldPos;
out vec2 iTextureCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    iNormal = mat3(transpose(inverse(uModel))) * aNormal;
    iTextureCoords = aTextureCoords;

    vec4 worldPos = uModel * vec4(aPosition, 1);
    iWorldPos = worldPos.xyz;

    gl_Position = uProjection * uView * worldPos;
}
