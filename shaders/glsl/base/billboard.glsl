uniform mat4 uBillboardMatrix;          // rotation matrix in world space
uniform vec2 uBillboardViewportSize;    // size of the billboard relative to viewport - i.e. <0.0, 1.0>
uniform vec3 uBillboardWorldPos;        // position of the billboard in the world

uniform vec2 uViewportSize;

uniform mat4 uView;
uniform mat4 uProjection;

vec4 CalculateBillboardPosition() 
{
    vec4 VertexClipPos = uProjection * uView * uBillboardMatrix * vec4(aPosition, 1);
    vec4 BillboardCenterClipPos = uProjection * uView * vec4(uBillboardWorldPos, 1.0);
    vec4 BillboardHalfSize = vec4((uViewportSize * uBillboardViewportSize) / uViewportSize / 2.0, 0, 0);

    return BillboardCenterClipPos + (BillboardHalfSize * VertexClipPos);
}