#include <espressif/esp_common.h>
#include <stdarg.h>

#include "config.h"

void logInfo(const char * msg)
{
#ifdef LOG_INFO
    printf("# %s", msg);
#endif
}

void logError(const char * msg)
{
#ifdef LOG_ERROR
    printf("/!\\ %s", msg);
#endif
}

void logDebug(const char * msg)
{
#ifdef LOG_DEBUG
    printf("* %s", msg);
#endif
}
