﻿#pragma once

#include "glad/glad.h"
#include "devices/gpu/fence.hpp"

namespace lucid::gpu
{
    class CGLFence : public CFence
    {
      public:
        CGLFence(const FString& InName, const GLsync& InGLFence);

        /** Fence interface */
        virtual void Wait(const u64& InTimeout) override;
        
        /** GPUObject interface */
        virtual void Free() override;
        virtual void SetObjectName() override;

      private:
        bool bSignaled = false;
        
        GLsync GLFenceHandle       = nullptr;
    };
} // namespace lucid::gpu