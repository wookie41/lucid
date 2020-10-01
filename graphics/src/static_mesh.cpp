#include "graphics/static_mesh.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/vao.hpp"

namespace lucid::graphics
{
    gpu::VertexArray* QuadVertexArray;
    StaticMesh QuadShape;

    static const float BASIC_QUAD_VERTEX_DATA[] 
    {
        //pos               //tex coords
        -1.0,  -1.0,  0.0,  0.0, 0.0,            //lower left
         1.0,  -1.0,  0.0,  1.0, 0.0,             //lower right
        -1.0,   1.0,  0.0,  0.0, 1.0,             //upper left
         1.0,   1.0,  0.0,  1.0, 1.0             //upper right
    };

    static const uint32_t BASIC_QUAD_ELEMENT_DATA[]
    {
        0, 1, 2, 3, 1 //triangle strip
    };

    void InitBasicShapes()
    {
        gpu::BufferDescription bufferDescription;

        //sending vertex data
        bufferDescription.data = (void*)(BASIC_QUAD_VERTEX_DATA);
        bufferDescription.size = sizeof(BASIC_QUAD_VERTEX_DATA);

        gpu::Buffer* QuadVertexBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);

        //sending element data
        bufferDescription.data = (void*)BASIC_QUAD_ELEMENT_DATA;
        bufferDescription.size = sizeof(BASIC_QUAD_ELEMENT_DATA);

        gpu::Buffer* QuadElementBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);

        StaticArray<gpu::VertexAttribute> quadAttributes(2);
        quadAttributes.Add({0, 3, Type::FLOAT, false, sizeof(float) * 5, 0, 0});
        quadAttributes.Add({1, 2, Type::FLOAT, false, sizeof(float) * 5, (sizeof(float) * 3), 0});
         
        QuadVertexArray = gpu::CreateVertexArray(&quadAttributes, QuadVertexBuffer, QuadElementBuffer, gpu::DrawMode::TRIANGLE_FAN, 4, 5);
        quadAttributes.Free();

        //we're not going to need the buffers on the CPU anymore, they have to be residient on the GPU tho, so we don't call Free()
        delete QuadVertexBuffer;
        delete QuadElementBuffer;

        QuadShape.VertexArray = QuadVertexArray;
    }

    void DrawMesh(const StaticMesh const* Mesh)
    {
        Mesh->VertexArray->Bind();
        Mesh->VertexArray->Draw();
        Mesh->VertexArray->Unbind();
    }
}