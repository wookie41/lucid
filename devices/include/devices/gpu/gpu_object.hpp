#pragma once
#include "gpu.hpp"
#include "common/strings.hpp"

namespace lucid::gpu
{
    class CGPUObject
    {
      public:
        CGPUObject(const FANSIString& InName, FGPUState* InGPUState) : Name(InName), GPUState(InGPUState) {}

        virtual void Free() = 0;
        virtual ~CGPUObject() = default;

        inline const FANSIString& GetName() const { return Name; }

      protected:
        const FANSIString& Name;

        /** GPUState for the context in which this object was created */
        FGPUState* GPUState;
    };

} // namespace lucid::gpu
