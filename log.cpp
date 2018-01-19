#include <espressif/esp_common.h>
#include <stdarg.h>

#include "web_server.h"
#include "config.h"

void log(const char * msg, ...)
{
    char logMsg[1024];
    va_list args;
    va_start(args, msg);
    sprintf(logMsg, msg, args);
    // sprintf(logMsg, "# %s", logMsg);
    printf(logMsg);
    // web_server_ws_send(logMsg);
}

void iflog(bool condition, const char * msg, ...)
{
    if (condition) {
        va_list args;
        va_start(args, msg);
        log(msg, args);
    }
}

void logInfo(const char * msg, ...)
{
#ifdef LOG_INFO
    va_list args;
    va_start(args, msg);
    log(msg, args);
#endif
}

void logError(const char * msg, ...)
{
#ifdef LOG_ERROR
    va_list args;
    va_start(args, msg);
    log(msg, args);
#endif
}

void logDebug(const char * msg, ...)
{
#ifdef LOG_DEBUG
    va_list args;
    va_start(args, msg);
    log(msg, args);
#endif
}
