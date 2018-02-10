#include <espressif/esp_common.h>
#include <stdarg.h>

#include "config.h"

void log(const char * prefix, const char * msg, ...)
{
    char logMsg[1024];
    va_list args;
    va_start(args, msg);
    vsprintf(logMsg, msg, args);
    va_end(args);
    sprintf(logMsg, "%s %s", prefix, logMsg);
    printf(logMsg);
}

void logInfo(const char * msg, ...)
{
#ifdef LOG_INFO
    va_list args;
    va_start(args, msg);
    log("#", msg, args);
    va_end(args);
#endif
}

void logError(const char * msg, ...)
{
#ifdef LOG_ERROR
    va_list args;
    va_start(args, msg);
    log("/!\\", msg, args);
    va_end(args);
#endif
}

void logDebug(const char * msg, ...)
{
#ifdef LOG_DEBUG
    va_list args;
    va_start(args, msg);
    log("*", msg, args);
    va_end(args);
#endif
}
