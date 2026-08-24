#pragma once

#include "MorphTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

bool morphPlatformOpenFileDialog(char* outPath, u32 outSize, const char* filter);

#ifdef __cplusplus
}
#endif