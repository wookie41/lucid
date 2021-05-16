#include "devices/gpu/gl/gl_timer.hpp"

#include "devices/gpu/gl/gl_common.hpp"

namespace lucid::gpu
{
    CGLTimer::CGLTimer(const FString& InName, const GLuint& InGLTimerQueryHandle) : CTimer(InName), GLTimerQueryHandle(InGLTimerQueryHandle) {}

    void CGLTimer::StartTimer()
    {
        assert(!bStarted);

        bStarted     = true;
        bStopped     = false;
        
        GLSyncHandle = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); // ensure that all prior commands have been executed

        glBeginQuery(GL_TIME_ELAPSED, GLTimerQueryHandle);
    };

    float CGLTimer::EndTimer()
    {
        assert(bStarted && !bStopped);

        bStopped = true;
        bStarted = false;

        glEndQuery(GL_TIME_ELAPSED);
        GLuint64 TimerResult;
        glGetQueryObjectui64v(GLTimerQueryHandle, GL_QUERY_RESULT, &TimerResult);
        glDeleteSync(GLSyncHandle);

        return float(TimerResult) / 1e06;
    };

    void CGLTimer::Free()
    {
        assert(GLTimerQueryHandle && bStopped);
        glDeleteQueries(1, &GLTimerQueryHandle);
        GLTimerQueryHandle = 0;
    }

    void CGLTimer::SetObjectName() { SetGLObjectName(GL_QUERY, GLTimerQueryHandle, Name); }

    CTimer* CreateTimer(const FString& InName)
    {
        GLuint TimerQuery;
        glGenQueries(1, &TimerQuery);
        assert(TimerQuery);
        return new CGLTimer(InName, TimerQuery);
    }
} // namespace lucid::gpu