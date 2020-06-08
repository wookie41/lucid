#pragma once

#include <cstdint>
#include "common/types.hpp"

namespace lucid::gpu
{
    void Init();

    struct VertexAttribute
    {
        uint32_t index;
        int32_t size;
        Type type;
        bool normalized;
        uint32_t stride;
        uint32_t offset;
        uint32_t divisor;
    };

    void AddVertexAttribute(const VertexAttribute& Attribute);
    void AddIntegerVertexAttribute(const VertexAttribute& Attribute);
    void AddLongVertexAttribute(const VertexAttribute& Attribute);
} // namespace lucid::gpu