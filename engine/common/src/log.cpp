#include "common/log.hpp"
#include "stdio.h"
#include <ctime>
#include <cstdarg>

namespace lucid
{
    static const char INFO_LEVEL_NAME[] = "INFO";
    static const char WARN_LEVEL_NAME[] = "WARN";
    static const char ERROR_LEVEL_NAME[] = "ERROR";

    static const char* LOG_LEVEL_NAMES[] = { INFO_LEVEL_NAME, WARN_LEVEL_NAME, ERROR_LEVEL_NAME };

    // dummy, temporary solution
    static char msgBuff[5096];

    //  static ExampleAppLog my_log;
    //  my_log.AddLog("Hello %d world\n", 123);
    //  my_log.Draw("title");
    void Log(const ELogLevel& InLevel, const char* InFile, const u32& InLine, const char* InFormat, ...)
    {
        va_list args;
        va_start(args, InFormat);
        vsprintf_s(msgBuff, 1024, InFormat, args);
        va_end(args);
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        printf("[%s] %d:%d:%d %s:%d - %s\n", (LOG_LEVEL_NAMES[static_cast<u8>(InLevel)]),
               now->tm_hour, now->tm_min, now->tm_sec, InFile, InLine, msgBuff);
    }
} // namespace lucid