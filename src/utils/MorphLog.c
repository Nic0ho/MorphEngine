#include "MorphLog.h"

static MorphOutputConsoleBuffer* sOutput = NULL;

void morphLog(LogType type, const char* fmt, ...)
{
    const char* prefix;
    switch (type)
    {
        case LOG_WARNING:
            prefix = "[WARNING] ";
            break;
        case LOG_ERROR:
            prefix = "[ERROR] ";
            break;
        default:
            prefix = "[MESSAGE] ";
            break;
    }

    char formatted[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(formatted, sizeof(formatted), fmt, args);
    va_end(args);

    printf("%s%s\n", prefix, formatted);

    if (sOutput != NULL)
    {
        char finalOutput[256];
        snprintf(finalOutput, sizeof(finalOutput), "%s%s", prefix, formatted);

        snprintf(sOutput->messages[sOutput->writeIndex], 256, "%s", finalOutput);

        sOutput->writeIndex++;
        if (sOutput->writeIndex >= MAX_CONSOLE_OUTPUT_LINES)
            sOutput->writeIndex = 0;
        
        sOutput->count++;
        if (sOutput->count >= MAX_CONSOLE_OUTPUT_LINES)
            sOutput->count = MAX_CONSOLE_OUTPUT_LINES;
    }
}

void morphLogSetOutput(MorphOutputConsoleBuffer* buffer)
{ sOutput = buffer; }