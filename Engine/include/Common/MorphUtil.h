#pragma once

#ifndef _WIN64
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <string.h>
#include <assert.h>
#include <time.h>
#ifndef MORPH_VULKAN
#endif
#include "MorphTypes.h"

#define ARRAY_SIZE_IN_BYTES(a) (sizeof(a[0]) * a.size())

extern char CurWorkDir[256];

bool ReadFile(const char* pFileName, std::string& outFile);
char* ReadBinaryFile(const char* pFileName, int& size);
void WriteBinaryFile(const char* pFileName, const void* pData, int size);