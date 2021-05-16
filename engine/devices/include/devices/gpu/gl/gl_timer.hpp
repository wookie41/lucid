#pragma once

#include <GL/glew.h>

#include "devices/gpu/timer.hpp"

namespace lucid::gpu
{
    class CGLTimer : public CTimer
    {
      public:
        CGLTimer(const FString& InName, const GLuint& InGLTimerQueryHandle);

        /** Timer interface */
        virtual void StartTimer() override;
        virtual float EndTimer() override;

        /** GPUObject interface */
        virtual void Free() override;
        virtual void SetObjectName() override;

      private:
        bool bStarted = false;
        bool bStopped = false;

        GLuint GLTimerQueryHandle = 0;
        GLsync GLSyncHandle       = nullptr;
    };
} // namespace lucid::gpu
