#pragma once

#include "common/types.hpp"

namespace lucid
{
    enum class ELogLevel : u8
    {
        INFO,
        WARN,
        ERR
    };

    void Log(const ELogLevel& InLevel, const char* InFile, const u32& InLine, const char* InFormat, ...);

#ifndef NDEBUG
#define LUCID_LOG(Level, Format, ...) Log(Level, __FILE__, __LINE__, Format, __VA_ARGS__);
#endif

#ifdef NDEBUG
#define LUCID_LOG(Level, Format, ...)
#endif

} // namespace lucid