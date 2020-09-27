#pragma once

#include <cstdint>

namespace lucid
{

    enum class LogLevel : uint8_t
    {
        INFO = 0,
        WARN,
        ERROR
    };

    void Log(const LogLevel& Level, const char* Message);


#ifndef NDEBUG
#define LUCID_LOG(Level, Message) Log((Level), (Message));
#endif

#ifdef NDEBUG
#define LUCID_LOG(Level, Message)
#endif

} // namespace lucid