#include "misc/basic_shapes.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/vao.hpp"

namespace lucid::misc
{
    gpu::VertexArray* QuadVertexArray;
    gpu::VertexArray* CubeVertexArray;

    static const float BASIC_QUAD_VERTEX_DATA[]{
        // pos         //normals //tangents      //tex coords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  25.0f, 25.0f

        // -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f, 25.0f,
        // -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f,
        //  25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  25.0f,  0.0f,

        //  25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  25.0f,  0.0f,
        //  25.0f, -0.5f,  -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  25.0f,  25.0f,
        // -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f, 25.0f
    };

    float CUBE_VERTICES[] = {
        // positions          // normals //tangents           // texture coords
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right          
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f  // bottom-left      
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

        StaticArray<gpu::VertexAttribute> quadAttributes(4);
        quadAttributes.Add({ 0, 3, Type::FLOAT, false, sizeof(float) * 11, 0, 0 });
        quadAttributes.Add({ 1, 3, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 3), 0 });
        quadAttributes.Add({ 2, 3, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 6), 0 });
        quadAttributes.Add({ 3, 2, Type::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 9), 0 });

        QuadVertexArray =
          gpu::CreateVertexArray(&quadAttributes, QuadVertexBuffer, nullptr, gpu::DrawMode::TRIANGLE_FAN, 6, 0);
        quadAttributes.Free();

        // we're not going to need the buffers on the CPU anymore, they have to be residient on the GPU tho, so we don't call
        // Free()
        delete QuadVertexBuffer;
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