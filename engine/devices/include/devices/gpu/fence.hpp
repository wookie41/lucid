#pragma once

#include "devices/gpu/gpu_object.hpp"

namespace lucid::gpu
{
    class CFence : public CGPUObject
    {
    public:

        CFence(const FString& InName) : CGPUObject(InName) {}

        virtual void Wait(const u64& InTimeout) = 0;
    };

    CFence* CreateFence(const FString& InName); 
}
