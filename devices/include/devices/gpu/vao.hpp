#pragma once

#include <cstdint>

#include "gpu_object.hpp"
#include "common/types.hpp"
#include "common/collections.hpp"

namespace lucid::gpu
{
    void Init(const u8& GLMajorVersion, const u8& GLMinorVersion);

    class CBuffer;

    struct FVertexAttribute
    {
        u32 Index = 0;
        int32_t NumComponents = 0;
        EType AttributeType;
        bool Normalized = false;
        u32 Stride = 0;
        u32 FirstElementOffset = 0;
        u32 Divisor = 0;
    };

    enum EDrawMode
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

    class CVertexArray : public CGPUObject
    {
      public:

        CVertexArray(const FANSIString& InName, FGPUState* InGPUState) : CGPUObject(InName, InGPUState) {}
        
        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void EnableAttribute(const u32& AttributeIndex) = 0;
        virtual void DisableAttribute(const u32& AttributeIndex) = 0;

        virtual void AddVertexAttribute(const FVertexAttribute& Attribute) = 0;
        virtual void AddIntegerVertexAttribute(const FVertexAttribute& Attribute) = 0;
        virtual void AddLongVertexAttribute(const FVertexAttribute& Attribute) = 0;

        virtual void SetVertexCount(const u32& Count) = 0;
        virtual u32 GetVertexCount() const = 0;

        virtual void SetElementCount(const u32& Count) = 0;
        virtual u32 GetElementCount() const = 0;

        // 'First' and 'Count' are 0 by default which basically means 'draw all'
        virtual void Draw(const u32& First = 0, const u32& Count = 0) = 0;
        virtual void DrawInstanced(const u32& InstancesCount,
                                   const u32& First = 0,
                                   const u32& Count = 0) = 0;

        virtual ~CVertexArray() = default;
    };
    CVertexArray* CreateVertexArray(const FANSIString& InName,
                                    FGPUState* InGPUState,
                                    FArray<FVertexAttribute>* VertexArrayAttributes,
                                    CBuffer* VertexBuffer,
                                    CBuffer* ElementBuffer,
                                    const EDrawMode& DrawMode,
                                    const u32& VertexCount,
                                    const u32& ElementCount,
                                    const bool& AutoDestroyBuffers = true);


} // namespace lucid::gpu