#include "misc/basic_shapes.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/gpu.hpp"
#include "devices/gpu/vao.hpp"

namespace lucid::misc
{
    static const float BASIC_QUAD_VERTEX_DATA[]{
        // pos         //normals //tangents      //tex coords
        -1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,
        -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,  0.0f,  0.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f
    };

    float CUBE_VERTICES[] = {
        // positions          // normals //tangents           // texture coords
            -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-right
             1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-right
            -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top-right
            // right face
            1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-right
            1.0f,  1.0f, -1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f, -1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f, -1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-right
            // bottom face
            -1.0f,  -1.0f, -1.0f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-left
             1.0f,  -1.0f , 1.0f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-right
             1.0f,  -1.0f, -1.0f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right     
             1.0f,  -1.0f,  1.0f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-right
            -1.0f,  -1.0f, -1.0f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-left
            -1.0f,  -1.0f,  1.0f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left      
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f  // bottom-left      
    };

    gpu::CVertexArray* CreateQuadVAO()
    {
        gpu::FBufferDescription bufferDescription;

        // sending vertex data
        bufferDescription.data = (void*)(BASIC_QUAD_VERTEX_DATA);
        bufferDescription.size = sizeof(BASIC_QUAD_VERTEX_DATA);

        gpu::CBuffer* QuadVertexBuffer = gpu::CreateBuffer(bufferDescription, gpu::EBufferUsage::STATIC, FSString {"QuadVertexBuffer"});

        FArray<gpu::FVertexAttribute> quadAttributes(4);
        quadAttributes.Add({ 0, 3, EType::FLOAT, false, sizeof(float) * 11, 0, 0 });
        quadAttributes.Add({ 1, 3, EType::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 3), 0 });
        quadAttributes.Add({ 2, 3, EType::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 6), 0 });
        quadAttributes.Add({ 3, 2, EType::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 9), 0 });

        gpu::CVertexArray* QuadVertexArray = gpu::CreateVertexArray(FSString {"QuadVertexArray" }, &quadAttributes, QuadVertexBuffer, nullptr, gpu::EDrawMode::TRIANGLE_STRIP, 4, 0);
        quadAttributes.Free();
        return QuadVertexArray;
    };

    gpu::CVertexArray* CreateCubeVAO()
    {
        gpu::FBufferDescription bufferDescription;

        // sending vertex data
        bufferDescription.data = (void*)(CUBE_VERTICES);
        bufferDescription.size = sizeof(CUBE_VERTICES);

        gpu::CBuffer* VertexBuffer = gpu::CreateBuffer(bufferDescription, gpu::EBufferUsage::STATIC,  FSString{ "CubeVertexBuffer"});

        FArray<gpu::FVertexAttribute> cubeAttributes(4);
        cubeAttributes.Add({ 0, 3, EType::FLOAT, false, sizeof(float) * 11, 0, 0 });
        cubeAttributes.Add({ 1, 3, EType::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 3), 0 });
        cubeAttributes.Add({ 2, 3, EType::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 6), 0 });
        cubeAttributes.Add({ 3, 2, EType::FLOAT, false, sizeof(float) * 11, (sizeof(float) * 9), 0 });

        gpu::CVertexArray* CubeVertexArray = gpu::CreateVertexArray(FSString {"CubeVertexArray" }, &cubeAttributes, VertexBuffer, nullptr, gpu::EDrawMode::TRIANGLES,
                                                 sizeof(CUBE_VERTICES) / (sizeof(float) * 11), 0);
        cubeAttributes.Free();

        return CubeVertexArray;
    };
} // namespace lucid::misc