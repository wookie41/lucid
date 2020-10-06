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

    void Log(const LogLevel& Level, const char* Format, ...);

#ifndef NDEBUG

#define LUCID_LOG(Level, Message, ...) Log(Level, Message, __VA_ARGS__);
#endif

#ifdef NDEBUG
#define LUCID_LOG(Level, Message, ...)
#endif

} // namespace lucid