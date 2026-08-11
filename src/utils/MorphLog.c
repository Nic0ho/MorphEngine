#include "MorphLog.h"

void morphLog(LogType type, const char* fmt, ...)
{
    switch (type)
    {
        case LOG_WARNING:
            printf("[WARNING] ");
            break;
        case LOG_ERROR:
            printf("[ERROR] ");
            break;
        default:
            printf("[MESSAGE] ");
            break;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}