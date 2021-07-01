uniform mat4 uBillboardMatrix;          // rotation matrix in world space
uniform vec2 uBillboardViewportSize;    // size of the billboard relative to viewport - i.e. <0.0, 1.0>
uniform vec3 uBillboardWorldPos;        // position of the billboard in the world

vec4 CalculateBillboardPosition() 
{
    vec4 BillboardCenterClipPos = uProjection * uView  * uBillboardMatrix * vec4(uBillboardWorldPos, 1.0);
    vec2 BillboardHalfSize = vec2(uBillboardViewportSize / 2.0 / (1/ BillboardCenterClipPos.z));
    
    return BillboardCenterClipPos + vec4(BillboardHalfSize * aPosition.xy, 0, 0);
}