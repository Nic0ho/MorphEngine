#pragma once

#ifndef _WIN64
#include <unistd.h>
#endif

#include <string>

#include <assert.h>

#ifndef MORPH_VULKAN
#endif
#include "MorphTypes.h"

#define ARRAY_SIZE_IN_BYTES(a) (sizeof(a[0]) * a.size())
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

extern char CurWorkDir[256];

bool ReadFile(const char* pFileName, std::string& outFile);
char* ReadBinaryFile(const char* pFileName, int& size);
void WriteBinaryFile(const char* pFileName, const void* pData, int size);