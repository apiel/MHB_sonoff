#include <espressif/esp_common.h>
#include <stdarg.h>

#include "config.h"

// need to improve

void logInfo(const char * msg, ...)
{
#ifdef LOG_INFO
    char logMsg[1024];
    va_list args;
    va_start(args, msg);
    vsprintf(logMsg, msg, args);
    va_end(args);
    printf("# %s", logMsg);
#endif
}

void logError(const char * msg, ...)
{
#ifdef LOG_ERROR
    char logMsg[1024];
    va_list args;
    va_start(args, msg);
    vsprintf(logMsg, msg, args);
    va_end(args);
    printf("/!\\ %s", logMsg);
#endif
}

void logDebug(const char * msg, ...)
{
#ifdef LOG_DEBUG
    char logMsg[1024];
    va_list args;
    va_start(args, msg);
    vsprintf(logMsg, msg, args);
    va_end(args);
    printf("* %s", logMsg);
#endif
}
