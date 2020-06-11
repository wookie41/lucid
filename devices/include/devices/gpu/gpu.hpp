#pragma once

#include <cstdint>
#include "common/types.hpp"
#include "common/collections.hpp"

namespace lucid::gpu
{
    void Init();

    class Buffer;

    struct VertexAttribute
    {
        uint32_t Index;
        int32_t Size;
        Type Type;
        bool Normalized;
        uint32_t Stride;
        uint32_t Offset;
        uint32_t Divisor;
    };

    void AddVertexAttribute(const VertexAttribute& Attribute);
    void AddIntegerVertexAttribute(const VertexAttribute& Attribute);
    void AddLongVertexAttribute(const VertexAttribute& Attribute);

    class VertexArray
    {
      public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void EnableAttribute(const uint32_t& AttributeIndex) = 0;
        virtual void DisableAttribute(const uint32_t& AttributeIndex) = 0;

        virtual ~VertexArray() = default;
    };

    VertexArray* CreateVertexArray(const StaticArray<VertexAttribute> const* VertexArrayAttributes,
                                   const Buffer const* VertexBuffer,
                                   const Buffer const* ElementBuffer);

    enum DrawMode
    {
        POINTS = 0,
        LINE_STRIP,
        LINE_LOOP,
        LINES,
        LINE_STRIP_ADJACENCY,
        LINES_ADJACENCY,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        TRIANGLES,
        TRIANGLE_STRIP_ADJACENCY,
        TRIANGLES_ADJACENCY,
        PATCHES
    };

    void DrawVertices(const uint32_t& First, const uint32_t& Count, const DrawMode& Mode);
    void DrawElement(const Type& IndiciesType, const uint32_t& Count, const DrawMode& Mode);
} // namespace lucid::gpu