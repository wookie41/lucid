#pragma once
#include "gpu.hpp"
#include "common/strings.hpp"

namespace lucid::gpu
{
    class CGPUObject
    {
      public:
        explicit CGPUObject(const FString& InName) : Name(InName) {};

        /* Sets the name on the GPU side */
        virtual void SetObjectName() = 0;
        
        virtual void Free() = 0;
        virtual ~CGPUObject() = default;

        inline const FString& GetName() const { return Name; }

      protected:
        const FString& Name;
    };

} // namespace lucid::gpu
