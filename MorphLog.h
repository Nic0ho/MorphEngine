#pragma once

#include "MorphTypes.h"

#include <stdarg.h>
#include <stdio.h>

typedef enum
{
    LOG_MESSAGE,
    LOG_WARNING,
    LOG_ERROR
} LogType;

void morphLog(LogType type, const char* fmt, ...);