#pragma once

#include "MorphTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

bool morphPlatformOpenFileDialog(char* outPath, u32 outSize, const char* filter);
bool morphPlatformSaveFileDialog(char* outPath, u32 outSize, const char* filter);
bool morphPlatformOpenFolderDialog(char* outPath, u32 outSize);
bool morphPlatformRemoveDirectory(const char* path);
bool morphPlatformRegisterFileAssociation(const char* exePath);

#ifdef __cplusplus
}
#endif