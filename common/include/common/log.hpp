#pragma once

#include "common/types.hpp"

namespace lucid
{
    enum class LogLevel : u8
    {
        INFO,
        WARN,
        ERR
    };

    void Log(const LogLevel& Level, const char* Format, ...);

#ifndef NDEBUG
#define LUCID_LOG(Level, Message, ...) Log(Level, Message, ##__VA_ARGS__);
#endif

#ifdef NDEBUG
#define LUCID_LOG(Level, Message, ...)
#endif

} // namespace lucid