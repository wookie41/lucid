#pragma once

#include "devices/gpu/gpu_object.hpp"

namespace lucid::gpu
{
    class CTimer : public CGPUObject
    {
    public:

        CTimer(const FString& InName) : CGPUObject(InName) {}

        virtual void StartTimer() = 0;

        /** Returns the number of miliseconds that passed on the GPU since StartTimer() was called */
        virtual float EndTimer() = 0;
    };

    CTimer* CreateTimer(const FString& InName); 
}
