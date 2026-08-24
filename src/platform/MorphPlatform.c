#include "MorphPlatform.h"
#include <windows.h>

bool morphPlatformOpenFileDialog(char* outPath, u32 outSize, const char* filter)
{
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = outPath;
    ofn.nMaxFile = outSize;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    return GetOpenFileNameA(&ofn) != 0;
}