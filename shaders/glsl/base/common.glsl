#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(bindless_sampler) uniform;
layout(bindless_image) uniform;

layout(std140, binding = 0) uniform GlobalDataBlock
{
    mat4      uProjection;
    mat4      uView;
    vec3      uViewPos;
    float     uAmbientStrength;
    vec2      uViewportSize;
    sampler2D uAmbientOcclusion;
    int       uNumPCFSamples;
    float     uParallaxHeightScale;
    float     uNearPlane;
    float     uFarPlane;
    int       uSSAOKernelSize;
    int       uSSAOStrength;
};