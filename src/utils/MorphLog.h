#pragma once

#include "MorphTypes.h"

#include <stdarg.h>
#include <stdio.h>

#define MAX_CONSOLE_OUTPUT_LINES 200

typedef enum
{
    LOG_MESSAGE,
    LOG_WARNING,
    LOG_ERROR
} LogType;

typedef struct
{
    char messages[MAX_CONSOLE_OUTPUT_LINES][256];
    LogType types[MAX_CONSOLE_OUTPUT_LINES];
    u32 count;
    u32 writeIndex;
} MorphOutputConsoleBuffer;

void morphLog(LogType type, const char* fmt, ...);
void morphLogSetOutput(MorphOutputConsoleBuffer* buffer);