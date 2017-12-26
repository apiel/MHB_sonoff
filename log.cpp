#include <espressif/esp_common.h>
#include <stdarg.h>

#include "httpd.h"

void log(char * msg, ...)
{
    char logMsg[1024];
    va_list args;
    va_start(args, msg);
    sprintf(logMsg, msg, args);
    printf(logMsg);
    ws_send_queue(logMsg);
}

void iflog(bool condition, char * msg, ...)
{
    if (condition) {
        va_list args;
        va_start(args, msg);
        log(msg, args);
    }
}
