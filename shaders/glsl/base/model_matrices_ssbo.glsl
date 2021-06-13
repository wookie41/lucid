#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_shader_storage_buffer_object : enable

uniform int uMeshBatchOffset;

layout(std430, binding = 0) buffer ModelMatricesDataBlock
{
    mat4 ModelMatrices[];
};

#define MODEL_MATRIX ModelMatrices[uMeshBatchOffset + gl_InstanceID]
