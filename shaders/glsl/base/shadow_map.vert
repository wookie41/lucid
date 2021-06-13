#version 450 core

#include "model_matrices_ssbo.glsl"

layout(location = 0) in vec3 aPosition;

uniform mat4 uLightMatrix;

void main() 
{
    gl_Position = uLightMatrix * MODEL_MATRIX * vec4(aPosition, 1.0);
}