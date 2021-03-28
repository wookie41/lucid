#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 uLightMatrices[6];

out vec4 FragPos;

void main()
{
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face;
        for (int vert = 0; vert < 3; vert++)
        {
            FragPos = gl_in[vert].gl_Position;
            gl_Position = uLightMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}