#include "MorphProject.h"
#include "MorphLog.h"
#include "MorphSerializer.h"
#include "MorphTypes.h"
#include <string.h>
#include <windows.h>

bool morphProjectCreate(MorphProject* project, const char* name, const char* location)
{
    char rootPath[MAX_PATH_LEN];
    snprintf(rootPath, sizeof(rootPath), "%s\\%s", location, name);

    if (!CreateDirectoryA(rootPath, NULL))
    {
        morphLog(LOG_ERROR, "Failed to create directory!");
        return false;
    }

    char engineContentPath[MAX_PATH_LEN];
    snprintf(engineContentPath, sizeof(engineContentPath), "%s\\%s\\Engine", location, name);

    if (!CreateDirectoryA(engineContentPath, NULL))
    {
        morphLog(LOG_ERROR, "Failed to create Engine content subfolder!");
        return false;
    }

    char projectContentPath[MAX_PATH_LEN];
    snprintf(projectContentPath, sizeof(projectContentPath), "%s\\%s\\Project", location, name);

    if (!CreateDirectoryA(projectContentPath, NULL))
    {
        morphLog(LOG_ERROR, "Failed to create Project content subfolder!");
        return false;
    }

    strncpy(project->name, name, 128);
    strncpy(project->rootPath, rootPath, MAX_PATH_LEN);

    char projectFilePath[MAX_PATH_LEN];
    snprintf(projectFilePath, sizeof(projectFilePath), "%s\\%s.mproj", rootPath, name);

    if (!morphProjectSave(project, projectFilePath))
    {
        morphLog(LOG_ERROR, "Failed to save the project!");
        return false;
    }

    return true;
}

bool morphProjectLoad(MorphProject* project, const char* filepath)
{
    MorphFile file = morphFileOpenRead(filepath);
    if (!file.isValid)
        return false;

    strncpy(project->rootPath, filepath, MAX_PATH_LEN);
    char* lastSlash = strrchr(project->rootPath, '\\');
    if (lastSlash) *lastSlash = '\0';
    
    fscanf(file.handle, "name=%127s", project->name);

    morphFileClose(&file);
    morphLog(LOG_MESSAGE, "Project loaded: %s from %s", project->name, project->rootPath);

    return true;
}

bool morphProjectSave(MorphProject* project, const char* filepath)
{
    MorphFile file = morphFileOpenWrite(filepath);
    if (!file.isValid)
        return false;

    fprintf(file.handle, "name=%s", project->name);

    morphFileClose(&file);
    morphLog(LOG_MESSAGE, "Project %s saved at destination: %s", project->name, filepath);

    return true;
}