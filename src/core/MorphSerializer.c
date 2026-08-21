#include "MorphSerializer.h"
#include "MorphLog.h"

MorphFile morphFileOpenWrite(const char* filepath)
{
    MorphFile result = {0};
    result.handle = fopen(filepath, "wb");

    if (result.handle != NULL)
    { result.isValid = true; }
    else
    {
        morphLog(LOG_ERROR, "Failed to open file for writing: %s", filepath);
        result.isValid = false;
    }

    return result;
}

MorphFile morphFileOpenRead(const char* filepath)
{
    MorphFile result = {0};
    result.handle = fopen(filepath, "rb");

    if (result.handle != NULL)
    { result.isValid = true; }
    else
    {
        morphLog(LOG_ERROR, "Failed to open file for reading: %s", filepath);
        result.isValid = false;
    }

    return result;
}

void morphFileClose(MorphFile* file)
{
    if (file->isValid && file->handle != NULL)
    {
        fclose(file->handle);
        file->isValid = false;
        file->handle = NULL;
    }
}

bool morphFileWrite(MorphFile* file, const void* data, usize size, usize count)
{
    if (!file->isValid || file->handle == NULL)
        return false; 
    
    usize written = fwrite(data, size, count, file->handle);
    
    return (written == count);
}

bool morphFileRead(MorphFile* file, void* data, usize size, usize count)
{
    if (!file->isValid || file->handle == NULL)
        return false;
    
    usize readCount = fread(data, size, count, file->handle);
    
    return (readCount == count);
}