#pragma once

#include "MorphTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char name[128];
    char rootPath[MAX_PATH_LEN];
    char enginePath[MAX_PATH_LEN];
    bool temporary;
} MorphProject;

bool morphProjectCreate(MorphProject* project, const char* name, const char* location, const char* enginePath);
bool morphProjectLoad(MorphProject* project, const char* filepath);
bool morphProjectSave(MorphProject* project, const char* filepath);
void morphProjectShutdown(MorphProject* project);

#ifdef __cplusplus
}
#endif