#pragma once

#include "MorphTypes.h"

typedef struct
{
    char name[128];
    char rootPath[MAX_PATH_LEN];
    bool temporary;
} MorphProject;

bool morphProjectCreate(MorphProject* project, const char* name, const char* location);
bool morphProjectLoad(MorphProject* project, const char* filepath);
bool morphProjectSave(MorphProject* project, const char* filepath);
void morphProjectShutdown(MorphProject* project);