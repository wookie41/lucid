#version 450 core

layout (location = 0) in vec3 aPosition;

uniform vec3 uViewPos;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 inNearPoint;
out vec3 inFarPoint;

void main() 
{
    // Unproject the clip-space grid coords to world space at near and far plane
    mat4 InvView = inverse(uView);
    mat4 InvProj = inverse(uProjection);

    vec4 NearPoint = InvView * InvProj * vec4(aPosition.x, aPosition.y, 0, 1);
    vec4 FarPoint = InvView * InvProj * vec4(aPosition.x, aPosition.y, 1, 1);

    inNearPoint = NearPoint.xyz / NearPoint.w;
    inFarPoint = FarPoint.xyz / FarPoint.w;

    gl_Position = vec4(aPosition, 1);
}


