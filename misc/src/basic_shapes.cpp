#include "misc/basic_shapes.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/vao.hpp"

namespace lucid::misc
{
    gpu::VertexArray* QuadVertexArray;
    gpu::VertexArray* CubeVertexArray;

    static const float BASIC_QUAD_VERTEX_DATA[]{
        // pos         //normals //tangents      //tex coords
        -1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, // lower left
        1.0,  -1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, // lower right
        -1.0, 1.0,  0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, // upper left
        1.0,  1.0,  0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0 // upper right
    };

    static const uint32_t BASIC_QUAD_ELEMENT_DATA[]{
        0, 2, 3, 1, 0 // triangle strip
    };

    float CUBE_VERTICES[] = {
        // positions          // normals //tangents           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0,   0.0,   0.0,   0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
        0.0,   0.0,   0.0,   1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0,   0.0,   0.0,   1.0f,  1.0f,  0.5f,
        0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0,   0.0,   0.0,   1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0,
        0.0,   0.0,   0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0,   0.0,   0.0,   0.0f,  0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0,   0.0,   0.0,   0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,
        0.0,   0.0,   0.0,   1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0,   0.0,   0.0,   1.0f,  1.0f,  0.5f,
        0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0,   0.0,   0.0,   1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0,
        0.0,   0.0,   0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0,   0.0,   0.0,   0.0f,  0.0f,

        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,
        0.0,   0.0,   0.0,   1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,  -0.5f,
        -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0,
        0.0,   0.0,   0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,
        0.0,   0.0,   0.0,   1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,  0.5f,
        -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0,
        0.0,   0.0,   0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
        0.0,   0.0,   0.0,   1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,  0.5f,
        -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0,
        0.0,   0.0,   0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
        0.0,   0.0,   0.0,   1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,  0.5f,
        0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0,   0.0,   0.0,   1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0,
        0.0,   0.0,   0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0,   0.0,   0.0,   0.0f,  1.0f
    };

    void InitQuad();
    void InitCube();

    void InitBasicShapes()
    {
        InitQuad();
        InitCube();
    }

    void InitQuad()
    {
        gpu::BufferDescription bufferDescription;

        // sending vertex data
        bufferDescription.data = (void*)(BASIC_QUAD_VERTEX_DATA);
        bufferDescription.size = sizeof(BASIC_QUAD_VERTEX_DATA);

        gpu::Buffer* QuadVertexBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);

        // sending element data
        bufferDescription.data = (void*)BASIC_QUAD_ELEMENT_DATA;
        bufferDescription.size = sizeof(BASIC_QUAD_ELEMENT_DATA);

        gpu::Buffer* QuadElementBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);

        StaticArray<gpu::VertexAttribute> quadAttributes(4);
        quadAttributes.Add({ 0, 3, Type::FLOAT, false, sizeof(float) * 11, 0, 0 });
        quadAttributes.Add({ 1, 3, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 3), 0 });
        quadAttributes.Add({ 2, 3, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 6), 0 });
        quadAttributes.Add({ 3, 2, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 9), 0 });

        QuadVertexArray =
          gpu::CreateVertexArray(&quadAttributes, QuadVertexBuffer, QuadElementBuffer, gpu::DrawMode::TRIANGLE_FAN, 4, 5);
        quadAttributes.Free();

        // we're not going to need the buffers on the CPU anymore, they have to be residient on the GPU tho, so we don't call
        // Free()
        delete QuadVertexBuffer;
        delete QuadElementBuffer;
    };

    void InitCube()
    {
        gpu::BufferDescription bufferDescription;

        // sending vertex data
        bufferDescription.data = (void*)(CUBE_VERTICES);
        bufferDescription.size = sizeof(CUBE_VERTICES);

        gpu::Buffer* VertexBuffer = gpu::CreateBuffer(bufferDescription, gpu::BufferUsage::STATIC);

        StaticArray<gpu::VertexAttribute> cubeAttributes(4);
        cubeAttributes.Add({ 0, 3, Type::FLOAT, false, sizeof(float) * 11, 0, 0 });
        cubeAttributes.Add({ 1, 3, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 3), 0 });
        cubeAttributes.Add({ 2, 3, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 6), 0 });
        cubeAttributes.Add({ 3, 2, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 9), 0 });

        CubeVertexArray = gpu::CreateVertexArray(&cubeAttributes, VertexBuffer, nullptr, gpu::DrawMode::TRIANGLES,
                                                 sizeof(CUBE_VERTICES) / (sizeof(float) * 11), 0);
        cubeAttributes.Free();

        // we're not going to need the buffers on the CPU anymore, they have to be residient on the GPU tho, so we don't call
        // Free()
        delete VertexBuffer;
    };
} // namespace lucid::misc