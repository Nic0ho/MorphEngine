#pragma once

#include "MorphTypes.h"
#include "MorphAssetType.h"
#include <stdio.h>

#define MORPH_MAGIC 0x4850524D

typedef struct
{
    FILE* handle;
    bool isValid;
} MorphFile;

typedef struct
{
    u32 magic;
    AssetType assetType;
} MorphAssetHeader;

MorphFile morphFileOpenWrite(const char* filepath);
MorphFile morphFileOpenRead(const char* filepath);
void morphFileClose(MorphFile* file);

bool morphFileWrite(MorphFile* file, const void* data, usize size, usize count);
bool morphFileRead(MorphFile* file, void* data, usize size, usize count);