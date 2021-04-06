#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 3) in vec2 aTextureCoords;

uniform mat4 uBillboardMatrix;          // rotation matrix in world space
uniform vec2 uBillboardViewportSize;    // size of the billboard relative to viewport - i.e. <0.0, 1.0>
uniform vec3 uBillboardWorldPos;        // position of the billboard in the world

uniform vec2 uViewportSize;

uniform mat4 uView;
uniform mat4 uProjection;

out vec2 oTextureCoords;

void main()
{
    vec4 VertexNDCPos = vec4(2.0 * aPosition - 1.0, 0);
    vec4 BillboardCenterClipPos = uProjection * uView * uBillboardMatrix * vec4(uBillboardWorldPos, 1.0);
    vec4 BillboardHalfSize = vec4((uViewportSize * uBillboardViewportSize) / uViewportSize / 2.0, 0, 0);

    oTextureCoords = aTextureCoords;
    gl_Position = BillboardCenterClipPos + (BillboardHalfSize * VertexNDCPos);
}
