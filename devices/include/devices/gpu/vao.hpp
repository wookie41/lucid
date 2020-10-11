#pragma once

#include <cstdint>
#include "common/types.hpp"
#include "common/collections.hpp"

namespace lucid::gpu
{
    void Init(const uint8_t& GLMajorVersion, const uint8_t& GLMinorVersion);

    class Buffer;

    struct VertexAttribute
    {
        uint32_t Index = 0;
        int32_t NumComponents = 0;
        Type AttributeType;
        bool Normalized = false;
        uint32_t Stride = 0;
        uint32_t FirstElementOffset = 0;
        uint32_t Divisor = 0;
    };

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

    class VertexArray
    {
      public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void EnableAttribute(const uint32_t& AttributeIndex) = 0;
        virtual void DisableAttribute(const uint32_t& AttributeIndex) = 0;

        virtual void AddVertexAttribute(const VertexAttribute& Attribute) = 0;
        virtual void AddIntegerVertexAttribute(const VertexAttribute& Attribute) = 0;
        virtual void AddLongVertexAttribute(const VertexAttribute& Attribute) = 0;

        virtual void SetVertexCount(const uint32_t& Count) = 0;
        virtual uint32_t GetVertexCount() const = 0;

        virtual void SetElementCount(const uint32_t& Count) = 0;
        virtual uint32_t GetElementCount() const = 0;

        // 'First' and 'Count' are 0 by default which basically means 'draw all'
        virtual void Draw(const uint32_t& First = 0, const uint32_t& Count = 0) = 0;
        virtual void DrawInstanced(const uint32_t& InstancesCount,
                                   const uint32_t& First = 0,
                                   const uint32_t& Count = 0) = 0;

        virtual void Free() = 0;

        virtual ~VertexArray() = default;
    };

    VertexArray* CreateVertexArray(StaticArray<VertexAttribute>* VertexArrayAttributes,
                                   Buffer* VertexBuffer,
                                   Buffer* ElementBuffer,
                                   const DrawMode& DrawMode,
                                   const uint32_t& VertexCount,
                                   const uint32_t& ElementCount,
                                   const bool& AutoDestroyBuffers = true);
} // namespace lucid::gpu