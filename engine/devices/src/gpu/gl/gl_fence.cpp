#include "devices/gpu/gl/gl_fence.hpp"
#include "devices/gpu/gl/gl_common.hpp"
#include "common/log.hpp"

namespace lucid::gpu
{
    CGLFence::CGLFence(const FString& InName, const GLsync& InGLFence) : CFence(InName), GLFenceHandle(InGLFence) {}

    bool CGLFence::Wait(const u64& InTimeout)
    {
        if (bSignaled)
        {
            return true;
        }

        GLenum Result = glClientWaitSync(GLFenceHandle, 0, InTimeout);
        if (Result == GL_ALREADY_SIGNALED || Result == GL_CONDITION_SATISFIED)
        {
            bSignaled = true;
        }
        else if (Result == GL_TIMEOUT_EXPIRED)
        {
            LUCID_LOG(ELogLevel::WARN, "Timer for fence %s expired", *Name);
            return false;
        }
        else if (Result == GL_WAIT_FAILED)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to wait for fence %s: error %d", *Name, glGetError());
            return false;
        }
        
        return bSignaled;
    }

    void CGLFence::Free() { glDeleteSync(GLFenceHandle); }

    void CGLFence::SetObjectName() {}

    CFence* CreateFence(const FString& InName)
    {
        GLsync SyncHandle;
        SyncHandle = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return new CGLFence(InName, SyncHandle);
    }
} // namespace lucid::gpu