#pragma once

#include "devices/gpu/gpu.hpp"

namespace lucid::graphics
{
    struct StaticMesh
    {
        bool IsIndexedDraw = false;
        uint32_t DrawOffset = 0;
        uint32_t DrawCount = 0;
        gpu::DrawMode DrawMode;
        gpu::VertexArray* VertexArray = nullptr;
    };

    extern StaticMesh QuadShape;

    void InitBasicShapes();
    void DrawMesh(const StaticMesh const* Mesh);
} // namespace lucid::graphics