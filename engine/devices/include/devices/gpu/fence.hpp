#pragma once

#include "devices/gpu/gpu_object.hpp"

namespace lucid::gpu
{
    class CFence : public CGPUObject
    {
      public:
        CFence(const FString& InName) : CGPUObject(InName) {}

        virtual bool Wait(const u64& InTimeout) = 0;
        inline bool  IsSignaled() const { return bSignaled; }

      protected:
        bool bSignaled = false;
    };

    CFence* CreateFence(const FString& InName);
} // namespace lucid::gpu
