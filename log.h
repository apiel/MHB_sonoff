
#ifndef __LOG_H__
#define __LOG_H__

void log(const char * prefix, const char * msg, ...);
void logInfo(const char * msg, ...);
void logError(const char * msg, ...);
void logDebug(const char * msg, ...);

#endif
