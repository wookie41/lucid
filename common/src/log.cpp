#include "common/log.hpp"
#include "stdio.h"
#include <ctime>

namespace lucid
{
    static const char INFO_LEVEL_NAME[] = "INFO";
    static const char WARN_LEVEL_NAME[] = "WARN";
    static const char ERROR_LEVEL_NAME[] = "ERROR";

    static const char* LOG_LEVEL_NAMES[] = { INFO_LEVEL_NAME, WARN_LEVEL_NAME, ERROR_LEVEL_NAME };

    void Log(const LogLevel& Level, const char* Message)
    {
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        printf("[%s] %d:%d:%d - %s\n", (LOG_LEVEL_NAMES[static_cast<uint8_t>(Level)]), now->tm_hour,
               now->tm_min, now->tm_sec, Message);
    }
} // namespace lucid